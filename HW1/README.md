# Huffman Coding
## Build
```
make BITS=<size of alphabet>
```
- Supported alphabet size: `8`, `16`, `32`, `64`
- `make clean` before changing the alphabet size
## Execution
```
./huffman -i <input-file> -o <output-file> [-e|-d] [-a <basic|adaptive>]
```
- `-a` for algorithm: `basic` (default), `adaptive`
- `-e` for encoding (default), `-d` for decoding