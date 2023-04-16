# SWPP Interpreter

This interpreter executes SWPP assembly programs.

## Build

- Requirement: cmake >= 3.10

```bash
# e.g. install cmake in ubuntu
sudo apt update
sudo apt install cmake
```

- Build

```bash
# creates "swpp-interpreter"
./build.sh
```

## Run

```bash
# executes a given assembly program and prints status to "swpp-interpreter.log"
# detailed information on the cost of the execution is emitted to "swpp-interpreter-cost.log"
# note that it gets a standard input on call to "read"
./swpp-interpreter <input assembly file>
```
