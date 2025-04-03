# Huffman Coding
## Auto Test
```
chmod +x test.sh
./test.sh <original-file> <result-file> <n_times>
```
Test all algorithms with all possible symbol size `n_times` times
## Build
```
make [BITS=<size-of-alphabet>]
```
- Supported alphabet size: `8`, `16`, `32` (default), `64` (not supported by adaptive huffman coding)
- `make clean` before changing the alphabet size
## Execution
```
./huffman -i <input-file> -o <output-file> [-e|-d] [-a <basic|adaptive>] [-p <file>]
```
- `-a` for algorithm: `basic` (default), `adaptive`
- `-e` for encoding (default), `-d` for decoding
- `-p` for output PMF information to the file specified (only supported for basic huffman encoding)
## PMF Chart
```
python3 draw.py <input-file> <output-folder>
```
- `input-file` is the file generate by `-p`