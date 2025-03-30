#ifndef __ADAPTIVE_HUFFMAN_H
#define __ADAPTIVE_HUFFMAN_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <set>
#include <stack>
#include <climits>
#include "defines.h"
using namespace std;

struct coding_tree_node
{
    uint64_t id;
    uint64_t weight;
    bool has_alphabet;
    alphabet c;
    coding_tree_node *parent;
    coding_tree_node *children[2];
};

extern coding_tree_node *root;

void init_coding_tree();
void adaptive_huffman(ifstream&, ofstream&);
void free_coding_tree(coding_tree_node *nd);
void adaptive_huffman_decode(ifstream&, ofstream&);

#endif