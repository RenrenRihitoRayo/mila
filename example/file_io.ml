
// ooen the file
let f = open("example.ml", "r+");

// go to the end
fseek(f, 0, SEEK_END);
// get the length
let size = ftell(f);

// go to beginning
fseek(f, 0, SEEK_SET);

// read the entire file
let text = fread(f, size);

// print it
println(text);

// go back to the beginning
fseek(f, 0, SEEK_SET);

// overerite file
fprint(f, text + "\n// comment!");

// close the file
fclose(f);