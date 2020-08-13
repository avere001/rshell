# Rshell
A lightweight shell written in C++ for my CS100 course

## Installing
To install rshell simply run the following commands in any terminal:
```
$ git clone  https://github.com/avere001/rshell.git
$ cd rshell
$ git checkout hw0
$ make
```

To run rshell simply run:
```
$ bin/rshell
```

## Usage
rshell can run any program located in `PATH` or your current directory.

The format for running a program is as follows:
```
$ command [argument 1] [argument 2] ...
```
arguments are whitespace seperated

### Connectors
Connectors can be used to run multiple commands conditionally.
The format is as follows:
```
$ command1 [arguments1] [connector] [command2] [arguments2] ...
```
#### Notes on syntax:
- Connectors need not be seperated from commands by any whitespace.
- Connectors must be immediately preceded by a command.
- Connectors followed immediately by another connector are invalid and will throw an error
- Connectors in the beginning of a line are invalid and will throw an error
- Connectors at the end of a line will simply be ignored.

#### Valid connectors 
- `;` -  runs the command following the connector always
- `&&` - runs the command following the connector only if the previously executed command resulted in a success
- `||` - runs the command following the connector only if the previously executed command resulted in a failure

#### Notes:
Commands are evaluated from left to right.
If a command doesn't exist or otherwise fails to execute, it will result in a failure.
The exception is that if a command is ignored because of a connector, the success is determined by the previously executed command.
Otherwise, If the command exists and successfully executes, then the success is determined by the return of the command (0 = success, anything else = failure)

#### Example:
```
$ true; echo command executed
command executed
$ false; echo command executed
command executed
$ true && echo command executed
command executed
$ false && echo command executed
$ true || echo command executed
$ false || echo command executed
command executed
$ false ; echo commands && echo can || echo not && echo be ; echo chained!
commands
can
be
chained!
```

### Output redirection
The output of a command may be redirected to a file like so:
```
$ echo abc123 > test
$ cat test
abc123
```

To append to an existing file use `>>` instead
```
$ echo abc > test
$ cat test
abc
$ echo abc > test
$ cat test
abc
$ echo 123 >> test
$ cat test
abc123
```

Additionally, you may specify the file descriptor to redirect by using `#>` or `#>>`

### Input redirection
You may use a file as input to a command like so:
```
$ echo abc > test
$ cat < test
abc
```

You may also use a string directly by using `<<<` instead
```
$ cat <<< test
test
```

### Piping
You may pipe the output of one command to the input of another like so:
```
$ echo test | cat
test
$ #you can chain them
$ echo test | tr a-z A-Z | cat
TEST
```

piping may be combined with redirection and connectors
```
$ echo test > abc
$ cat < abc | cat > def ; cat < def | tr a-z  A-Z | cat
TEST
```

### Comments
Any text on a line following a `#` will be ignored.

#### Example:
```
$ echo this won't be ignored #but this will!
this won't be ignored
```

### Quitting rshell
In order to properly close rshell, simply type exit just like any other command.
exit will ignore any arguments passed to it. Additionally any commands connected after exit is called will not be executed
```
$ exit
```

## Builtins
currently the only shell builtin is cd.

### cd
cd can be used to change directories

#### Usage

cd home:
```
$ cd
```

cd to path.
this can be relative or absolute:
```
$ cd /
$ cd bin
```

finally, to go up a directory:
```
cd dir1
cd ../dir2
cd - #will take you to dir1
``` 

## Signals
Currently only supports SIGINT

send SIGINT to a process by pressing CTRL+C in any running program

## Bugs:
- There is no way to pass `;`, `&&`, `||`, `&`, `|`, `#`, `>`, `<`,  or whitespace as arguments
- using >>> will redirect to a file called > rather than error
- You cannot redirect the output of cd -
