#!/usr/bin/env python3
import os
import shlex
import json
import sys
import re
from pathlib import Path
from typing import Optional, List, Dict, Any

class JsonRpcServer:
    def __init__(self):
        self.msg_id = 0
    
    def read_message(self):
        headers = {}
        while True:
            line = sys.stdin.readline()
            if not line:
                return None
            line = line.rstrip('\r\n')
            if not line:
                break
            key, val = line.split(':', 1)
            headers[key] = val.strip()
        
        size = int(headers.get('Content-Length', 0))
        msg = sys.stdin.read(size)
        return json.loads(msg)
    
    def send_message(self, msg):
        body = json.dumps(msg)
        sys.stdout.write(f"Content-Length: {len(body)}\r\n\r\n{body}")
        sys.stdout.flush()
    
    def send_response(self, msg_id, result):
        self.send_message({"jsonrpc": "2.0", "id": msg_id, "result": result})
    
    def send_error(self, msg_id, code, message):
        self.send_message({"jsonrpc": "2.0", "id": msg_id, "error": {"code": code, "message": message}})
    
    def send_notification(self, method, params):
        self.send_message({"jsonrpc": "2.0", "method": method, "params": params})

class MTagsParser:
    def __init__(self):
        self.symbols: Dict[str, List[Dict[str, Any]]] = {}
        self.mtags_path: Optional[Path] = None
        
    def find_mtags(self, start_path: Path) -> Optional[Path]:
        max_dir_up = int(os.getenv("MILA_LSP_MAX_DIR_UP", "20"))
        current = start_path.resolve()
        for _ in range(max_dir_up):
            mtags = current / "MTAGS"
            if mtags.exists():
                return mtags
            if current.parent == current:
                break
            current = current.parent
        return None
    
    def parse_mtags(self, path: Path) -> None:
        self.symbols.clear()
        try:
            with open(path, 'r') as f:
                for line in f:
                    line = line.strip()
                    if not line or line.startswith('#'):
                        continue
                    self._parse_line(line)
            self.mtags_path = path
            log_message(f"Loaded MTAGS from {path}, found {len(self.symbols)} symbols")
        except Exception as e:
            log_message(f"Error parsing MTAGS: {e}")
    
    def _parse_line(self, line: str) -> None:
        parts = line.split(None, 6)
        decl = parts[6]
        parts = shlex.split(line)[:6]
        log_message(f"{decl} | {line} | {line.split(None, 6)}")
        if len(parts) != 6 or not decl:
            return 
        try:
            location = parts[0]
            scope_depth = int(parts[1])
            scope_id = int(parts[2])
            stmt_type = parts[3]
            type_val = parts[4]
            name = parts[5]
            
            if ':' not in location:
                return
            
            file_part, line_num, col_num = location.rsplit(':', 2)
            
            symbol = {
                "name": name,
                "type": type_val,
                "stmt_type": stmt_type,
                "file": file_part,
                "line": int(line_num),
                "col": int(col_num),
                "scope_depth": scope_depth,
                "scope_id": scope_id,
                "decl": decl
            }
            
            if name not in self.symbols:
                self.symbols[name] = []
            self.symbols[name].append(symbol)
        except (ValueError, IndexError) as e:
            log_message(f"Failed to parse line: {line} - {e}")

parser = MTagsParser()
server = JsonRpcServer()
open_docs = {}
debug_log = []
CWD = []

def log_message(msg):
    """Log for debugging"""
    debug_log.append(msg)
    if len(debug_log) > 100:
        debug_log.pop(0)
    
    # Also write to file so we can see what's happening
    try:
        log_file = Path.home() / "mtags_lsp.log"
        with open(log_file, 'a') as f:
            f.write(f"{msg}\n")
            f.flush()
    except:
        pass

def path_resolve(file):
    for fdir in CWD:
        if os.path.exists(file_dir:=os.path.join(fdir, file)):
            return file_dir
    return file

def handle_message(msg):
    method = msg.get("method")
    params = msg.get("params", {})
    msg_id = msg.get("id")
    
    log_message(f"Received: {method}")
    
    if method == "initialize":
        server.send_response(msg_id, {
            "capabilities": {
                "textDocumentSync": 2,  # Full document sync
                "completionProvider": {
                    "resolveProvider": False,
                    "triggerCharacters": ["."]
                },
                "hoverProvider": True,
                "definitionProvider": True
            },
            "serverInfo": {
                "name": "mtags-lsp",
                "version": "1.1.0"
            }
        })
    
    elif method == "initialized":
        # LSP requires response to initialized notification
        log_message("Client initialized")
    
    elif method == "textDocument/didOpen":
        uri = params.get("textDocument", {}).get("uri", "")
        text = params.get("textDocument", {}).get("text", "")
        open_docs[uri] = text
        log_message(f"Opened: {uri}")
        
        # Try to find MTAGS
        path = Path(uri.replace("file://", ""))
        CWD.append(path.parent)
        mtags = parser.find_mtags(path.parent)
        if mtags:
            parser.parse_mtags(mtags)
            log_message(f"Found MTAGS: {mtags}")
        else:
            log_message(f"No MTAGS found from {path.parent}")
    
    elif method == "textDocument/didChange":
        uri = params.get("textDocument", {}).get("uri", "")
        for change in params.get("contentChanges", []):
            open_docs[uri] = change.get("text", "")
        server.send_response(msg_id, None)
    
    elif method == "textDocument/completion":
        log_message(f"Completion request: mtags_path={parser.mtags_path}, symbols count={len(parser.symbols)}")
        
        if not parser.mtags_path:
            log_message("No MTAGS loaded, returning empty completions")
            server.send_response(msg_id, [])
            return
        
        uri = params.get("textDocument", {}).get("uri", "")
        line = params.get("position", {}).get("line", 0)
        char = params.get("position", {}).get("character", 0)
        
        log_message(f"Completion at {uri}:{line}:{char}")
        
        prefix = ""
        if uri in open_docs:
            lines = open_docs[uri].split('\n')
            if line < len(lines):
                current_line = lines[line][:char]
                match = re.search(r'\b(\w*)$', current_line)
                if match:
                    prefix = match.group(1)
                log_message(f"Found prefix: '{prefix}' from line: {repr(current_line)}")
        
        items = []
        for name in sorted(parser.symbols.keys()):
            if prefix and not name.startswith(prefix):
                continue

            sym = parser.symbols[name][0]

            items.append({
                "label": name,
                "kind": "value" if sym['stmt_type'] != "fn" else "function",
                "detail": f"{sym['stmt_type']}: {sym['type']}",
                "documentation": sym.get("decl", "")
            })

        server.send_response(msg_id, items)
    
    elif method == "textDocument/hover":
        if not parser.mtags_path:
            server.send_response(msg_id, None)
            return
        
        uri = params.get("textDocument", {}).get("uri", "")
        line = params.get("position", {}).get("line", 0)
        char = params.get("position", {}).get("character", 0)
        
        if uri in open_docs:
            lines = open_docs[uri].split('\n')
            if line < len(lines):
                current_line = lines[line]
                # Extract word at cursor
                start = char
                while start > 0 and (current_line[start-1].isalnum() or current_line[start-1] == '_'):
                    start -= 1
                end = char
                while end < len(current_line) and (current_line[end].isalnum() or current_line[end] == '_'):
                    end += 1
                word = current_line[start:end]
                
                if word in parser.symbols:
                    sym = parser.symbols[word][0]
                    if sym["stmt_type"] != "fn":
                        content = f"**{sym['stmt_type']}** `{word}`: {sym['type']}\n\n```MiLa\n{sym.get('decl', '"No Declaration found!"')}\n```"
                    else:
                        content = f"Function `{sym['name']}`.\nReturns\n```MiLa\n{sym['type']}\n```\n\n```MiLa\n{sym.get('decl', '"No Declaration found!"')}\n```"
                    log_message(f"Hover for '{word}'...")
                    server.send_response(msg_id, {
                        "contents": {
                            "kind": "markdown",
                            "value": content
                        }
                    })
                    log_message(f"Sent hover response for '{word}'")
                    return
        
        server.send_response(msg_id, None)
    
    elif method == "textDocument/definition":
        if not parser.mtags_path:
            server.send_response(msg_id, None)
            return
        
        uri = params.get("textDocument", {}).get("uri", "")
        line = params.get("position", {}).get("line", 0)
        char = params.get("position", {}).get("character", 0)
        
        if uri in open_docs:
            lines = open_docs[uri].split('\n')
            if line < len(lines):
                current_line = lines[line]
                start = char
                while start > 0 and (current_line[start-1].isalnum() or current_line[start-1] == '_'):
                    start -= 1
                end = char
                while end < len(current_line) and (current_line[end].isalnum() or current_line[end] == '_'):
                    end += 1
                word = current_line[start:end]
                
                if word in parser.symbols:
                    sym = parser.symbols[word][0]
                    server.send_response(msg_id, {
                        "uri": f"file://{path_resolve(os.path.basename(sym['file']))}",
                        "range": {
                            "start": {"line": sym['line']-1, "character": sym['col']},
                            "end": {"line": sym['line']-1, "character": sym['col'] + len(word)}
                        }
                    })
                    return
        
        server.send_response(msg_id, None)
    
    elif method == "shutdown":
        server.send_response(msg_id, None)
        sys.exit(0)

if __name__ == "__main__":
    # Clear log and write startup message
    try:
        log_file = Path.home() / "mtags_lsp.log"
        with open(log_file, 'w') as f:
            f.write("=== LSP Server Starting ===\n")
    except:
        pass
    
    log_message("LSP Server started")
    log_message(f"Python version: {sys.version}")
    log_message(f"Working directory: {os.getcwd()}")
    
    while True:
        try:
            msg = server.read_message()
            if msg is None:
                log_message("No more messages, exiting")
                break
            handle_message(msg)
        except Exception as e:
            log_message(f"Exception: {e}")
            import traceback
            log_message(traceback.format_exc())
