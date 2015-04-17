# rshell
A lightweight shell written in CS100

## Installing


## Usage
rshell can run any program located in `PATH`.
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
Notes on syntax:
- Connectors need not be seperated from commands by any whitespace.
- Connectors must be immediately preceded by a command.
- Connectors followed immediately by another connector are invalid and will throw an error
- Connectors in the beginning of a line are invalid and will throw an error
- Connectors at the end of a line will simply be ignored.

- `;` -  runs the command following the connector always
- `&&` - runs the command following the connector only if the previously executed command resulted in a success
- `||` - runs the command following the connector only if the previously executed command resulted in a failure

Note:
Command are evaluated from left to right.
If a command doesn't exist or otherwise fails to execute, it will result in a failure.
The exception is that if a command is ignored because of a connector, the success is determined by the previously executed command.
Otherwise, If the command exists and successfully executes, then the success is determined by the return of the command (0 = success, anything else = failure)

example:
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

### Comments
Any text on a line following a `#` will be ignored.

example:
```
$ echo this won't be ignored #but this will!
this won't be ignored
```

### Quitting rshell
In order to properly close rshell, simply type exit just like any other command.
exit will ignore any arguments passed to it
```
$ exit
```





## Bugs:
There is no way to pass `;`, `&&`, `||`, `&`, `|`, `#`, or whitespace as arguments
There is no way to change current working directory
