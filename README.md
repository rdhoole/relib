# relib
A configurable dynamic library for hooking functions to repurpose them.

### Features
As of version 1.0:
1. `getline(...)` function has been implemented to be hooked.
2. Function(s) implemented should be logged to a file specified in `options.h`
3. Embedded scripting using Python. (version 1.0 uses python3.7)

### Compile Options
`options.h` will have the defines that will determine what will be built in the compiled library. 0 is exclude and 1 (or any number) is include. Examining `relib.c` will show exactly what will be built in.

### Build
To compile, use `make`</br>
`make clean` to clean directory.

### Use
Once you have all the dependencies met to compile your options, you will use this like any other lib hook:</br></br>
`$ LD_PRELOAD=./relib.so ./program`

### Embedded Python Scripting
Given the proper delimiter `>>>`, all input after will be sent to the python interpreter.</br></br>
Example on a program with getline (GETLINE, PYTHON and LOG_FILE):
```bash
$ LD_PRELOAD=./relib.so ../simpledb
db > test
Unrecognized keyword at start of 'test'
db > >>>f=open("test.txt","w");f.write("Hello World");f.close()
Unrecognized keyword at start of '>>>f=open("test.txt","w");f.write("Hello World\n");f.close()'
db > .exit
$ cat test.txt
Hello World
$ cat log.dat
getline(char **lineptr, size_t *n, FILE *stream):
 : test
getline(char **lineptr, size_t *n, FILE *stream):
 : >>>f=open("test.txt","w");f.write("Hello World");f.close()
```
