#!/bin/sh

if [ $# -ne 3 ]; then
    echo "usage: ./test.sh <original-file> <result-file> <n_times>"
    exit 1
fi

orig="$1"
rst="$2"
tmp=".tmp"
times="$3"

echo "---------- Test Basic Huffman Coder ----------"

for i in 8 16 32 64; do
    echo "----- Test $i-bit -----"
    make clean
    make BITS="$i"
    for j in $(seq 1 $times); do
        echo "--- $j ---"
        echo "--- Encode ---"
        /usr/bin/time -alp ./huffman -i "$orig" -o "$tmp" -e -a basic
        echo "--- Decode ---"
        /usr/bin/time -alp ./huffman -i "$tmp" -o "$rst" -d -a basic
    done
    diff "$orig" "$rst"
    if [ $? -eq 0 ]; then
        echo "----- Matches the original file -----"
        echo "--- Compression Ratio: $(echo "scale=4; $(wc -c < "$tmp") * 100 / $(wc -c < "$orig")" | bc)% ("$(wc -c < "$tmp")" /"$(wc -c < "$orig")" ) ---"
    else
        echo "----- Doesn't match the original file -----"
    fi
done

echo "---------- Test Adaptive Huffman Coder ----------"

for i in 8 16 32; do
    echo "----- Test $i-bit -----"
    make clean
    make BITS="$i"
    for j in $(seq 1 $times); do
        echo "--- $j ---"
        echo "--- Encode ---"
        /usr/bin/time -alp ./huffman -i "$orig" -o "$tmp" -e -a adaptive
        echo "--- Decode ---"
        /usr/bin/time -alp ./huffman -i "$tmp" -o "$rst" -d -a adaptive
    done
    truncate -s $(wc -c < "$orig") "$rst"
    diff "$orig" "$rst"
    if [ $? -eq 0 ]; then
        echo "----- Matches the original file -----"
        echo "--- Compression Ratio: $(echo "scale=4; $(wc -c < "$tmp") * 100 / $(wc -c < "$orig")" | bc)% ("$(wc -c < "$tmp")" /"$(wc -c < "$orig")" ) ---"
    else
        echo "----- Doesn't match the original file -----"
    fi
done

make clean
rm "$tmp"