# Huffman Coding
## Build
```
make BITS=<size of alphabet> [ALGO=<algorithm>]
```
- Supported alphabet size: `8`, `16`, `32`, `64`
- Supported algorithm: `BASIC` (basic huffman, default), `ADAPTIVE` (adaptive huffman)
- `make clean` before changing the algorithm or alphabet size
## Execution
```
./huffman <input file> <output file>
```