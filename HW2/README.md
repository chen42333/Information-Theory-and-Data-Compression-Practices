# Context-based Binary Arithmetic Coding
## Build
```
make
```
## Execution
```
./ac -i <input-file> -o <output-file> -c <natural|unary> -p <fix|ppm> [-n <PPM-order>]
```
- `-c` for binarization algorithm: `basic` (default), `adaptive`
- `-p` for probability model
- `-n` for the max context order for PPM (default=2)