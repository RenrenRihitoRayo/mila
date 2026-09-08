#include "mila.h"
#include <unistd.h>
#include <sys/mount.h>
#include "ml_commons.h"

#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/wait.h>

Value* native_mll_get_errno(Env* env, int argc, Value** argv) {
    return vint(errno);
}

Value* native_mll_strerror(Env* env, int argc, Value** argv) {
    if (argc != 1 || GET_TYPE(argv[0]) != T_INT) return verror("strerror(e): Expected a single int.");
    return vstring_dup(strerror(GET_INTEGER(argv[0])));
}

Value* native_mll_reboot(Env* env, int argc, Value** argv) {
    sync();
    reboot(LINUX_REBOOT_CMD_RESTART);
    return vnull();
}

Value* native_mll_shutdown(Env* env, int argc, Value** argv) {
    sync();
    reboot(LINUX_REBOOT_CMD_POWER_OFF);
    return vnull();
}

Value* native_mll_fork(Env* env, int argc, Value** argv) {
    return vint(fork());
}

Value *native_mll_wait(Env* env, int argc, Value** argv) {
    int status = 0;
    int pid = wait(&status);
    return make_list(vint(status), vint(pid), NULL);
}

Value* native_mll_chdir(Env* env, int argc, Value** argv) {
    if (argc != 1 || GET_TYPE(argv[0]) != T_STRING) return verror("chdir(dir): Expected one string argument!");
    return vint(chdir(GET_STRING(argv[0])));
}

Value* native_mll_mount(Env* env, int argc, Value** argv) {
    if (GET_TYPE(argv[0]) != T_STRING || GET_TYPE(argv[1]) != T_STRING || GET_TYPE(argv[2]) != T_STRING || GET_TYPE(argv[3]) != T_INT || argc < 3)
        return verror("mount(src, dst, flags, data): Expected at least 4 valid arguments!");
    if (argc >= 5 || (argc == 5 && GET_TYPE(argv[4]) != T_STRING))
        return verror("mount(src, dst, fs_type, flags, data): data argument must be a string or left out from the call!");
    return vint(mount(GET_STRING(argv[0]), GET_STRING(argv[1]), GET_STRING(argv[2]), GET_INTEGER(argv[3]), argc == 5 ? GET_STRING(argv[4]) : NULL));
}

Value* native_mll_execve(Env* env, int argc, Value** argv) {
    if (argc != 3 || strcmp(GET_TYPENAME(argv[1]), "list") != 0 || strcmp(GET_TYPENAME(argv[2]), "list") != 0 || GET_TYPE(argv[0]) != T_STRING)
        return verror("execve(name, argv, envp): Invalid arguments!");
    struct {
        char** items;
        size_t size, count;
    } cmd_argc = {0};
    struct {
        char** items;
        size_t size, count;
    } cmd_envp = {0};
    ITERATE_LIST((LinkedList*)GET_OPAQUE(argv[1])) { 
        da_append(&cmd_argc, GET_STRING(current->value));
        printf("argc[] = %s\n", GET_STRING(current->value));
    }
    ITERATE_LIST((LinkedList*)GET_OPAQUE(argv[2])) {
        da_append(&cmd_envp, GET_STRING(current->value));
        printf("envp[] = %s\n", GET_STRING(current->value));
    }
    da_append(&cmd_argc, NULL);
    da_append(&cmd_envp, NULL);
    printf("argc=%ld, envp=%ld\n", cmd_argc.count, cmd_envp.count);
    execve(GET_STRING(argv[0]), cmd_argc.items, cmd_envp.items);
    return vnull();
}

void env_register_linux(Env* g) {
    env_register_native(g, "get_errno", native_mll_get_errno);
    env_register_native(g, "strerror", native_mll_strerror);
    env_register_native(g, "mount", native_mll_mount);
    env_register_native(g, "execve", native_mll_execve);
    env_register_native(g, "fork", native_mll_fork);
    env_register_native(g, "wait", native_mll_wait);
    env_register_native(g, "shutdown", native_mll_shutdown);
    env_register_native(g, "reboot", native_mll_reboot);
}
