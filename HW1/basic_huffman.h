#ifndef __BASIC_HUFFMAN_H
#define __BASIC_HUFFMAN_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <tuple>
#include <queue>
#include <stack>
#include <climits>

#if defined(__APPLE__) // macOS
    #include <malloc/malloc.h>
#elif defined(__linux__) // Linux
    #include <malloc.h>
#endif

#include "defines.h"
using namespace std;

#define TABLE_SIZE_EXP CHAR_BIT

struct prob_node
{
    alphabet c;
    double p;
    bool is_set;
    bool operator < (const prob_node &nd) const
    {
        return this->p > nd.p;
    }
};

struct code_node
{
    uint64_t len; // The unit is bit
    uint8_t code[];
};

struct code_table_node // For decoding
{
    alphabet c;
    uint64_t len; // The unit is bit
};

extern void *code_table[];

void count_alphabet(ifstream&);
void huffman(unordered_map<alphabet, tuple<alphabet, alphabet, double>>&, unordered_map<alphabet, tuple<alphabet, alphabet>>&);
void free_code_table(void**, int);
void fill_code_table(unordered_map<alphabet, tuple<alphabet, alphabet, double>>&, unordered_map<alphabet, tuple<alphabet, alphabet>>&);
void output(ifstream&, ofstream&);

void fill_code_table_decode(ifstream&);
void huffman_decode(ifstream&, ofstream&);
void free_code_table_decode(void**);

void output_pmf_info(ifstream&, ofstream&);

#endif