#ifndef __BASIC_HUFFMAN_H
#define __BASIC_HUFFMAN_H

#include <iostream>
#include <fstream>
#include <map>
#include <tuple>
#include <queue>
#include <stack>
#include <climits>
#include "defines.h"
using namespace std;

#define TABLE_SIZE_EXP 8

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

extern void *code_table[];

void count_alphabet(ifstream&);
void huffman(map<alphabet, tuple<alphabet, alphabet>>&, map<alphabet, tuple<alphabet, alphabet>>&);
void free_code_table(void**, int);
void fill_code_table(map<alphabet, tuple<alphabet, alphabet>>&, map<alphabet, tuple<alphabet, alphabet>>&);
void output(ifstream&, ofstream&);

#endif