#ifndef __ADAPTIVE_HUFFMAN_H
#define __ADAPTIVE_HUFFMAN_H

#include <iostream>
#include <fstream>
#include "defines.h"
using namespace std;

void init_coding_tree();
void adaptive_huffman(ifstream&, ofstream&);

#endif