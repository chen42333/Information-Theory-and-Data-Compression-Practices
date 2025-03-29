#include <iostream>
#include <fstream>
#include "defines.h"

#ifdef BASIC
#include "basic_huffman.h"
#elif defined(ADAPTIVE)
#include "adaptive_huffman.h"
#else
#error "Unsupported algorithm"
#endif

using namespace std;

inline static void usage()
{
    cout << "usage: ./huffman <input-file> <output-file>" << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        usage();
        return 0;
    }

    char *input_fname = argv[1], *output_fname = argv[2];
    ifstream input_file(input_fname, ios::binary);
    ofstream output_file(output_fname, ios::binary);
#ifdef BASIC
    map<alphabet, tuple<alphabet, alphabet>> node; // The tuple stands for (set, bit)
    map<alphabet, tuple<alphabet, alphabet>> set;

    count_alphabet(input_file);
    huffman(node, set);

    input_file.clear();
    input_file.seekg(0, std::ios::beg);

    fill_code_table(node, set);
    output(input_file, output_file);

    free_code_table(code_table, 0);
#elif defined(ADAPTIVE)
    init_coding_tree();
    adaptive_huffman(input_file, output_file);
#else
#error "Unsupported algorithm"
#endif

    input_file.close();
    output_file.close();

    return 0;
};