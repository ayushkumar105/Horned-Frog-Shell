# Horned Frog Shell 2

Horned Frog Shell 2 is a custom Linux shell, named after the cherished mascot of our school. This shell does not use the `system()` function to execute commands, instead, it directly invokes the necessary binary files, providing a more integrated and efficient shell experience.

The unique feature of this shell is its custom output redirection. When a user types a command like `ls -la /tmp > output`, nothing is printed on the screen. Instead, both the standard output and the standard error output of the `ls` program are redirected to the file named `output`.

## Features

- Executes most Linux commands directly, without opening another shell.
- Custom output redirection.

## Installation

Clone this repository to your local machine using:

```bash
git clone https://github.com/ayushkumar105/Horned-Frog-Shell.git
```
## How to Use

To build the shell, navigate to the directory containing the source files, and run:

```bash
make
```

After the shell is built, you can start it in interactive mode with:

```bash
./hfsh2
```

Or you can run a bash script by specifying the script's path as an argument:

```bash
./hfsh2 path_to_your_script
```

Commands can be entered at the prompt in interactive mode. For example, to list all the files in the `/tmp` directory and redirect the output to a file named `output`, you would type:

```bash
ls -la /tmp > output
```

This will not display any output on the screen. Instead, both the standard output and the standard error output will be redirected to the file `output`.

## Acknowledgements

Thanks to the Department of Computer Science at Carnegie Mellon University for providing the csapp.c and csapp.h files, which have been instrumental in the development of this project.
