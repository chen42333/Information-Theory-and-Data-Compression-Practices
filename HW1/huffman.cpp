#include <iostream>
#include <fstream>
#include <unistd.h>
#include "defines.h"
#include "basic_huffman.h"
#include "adaptive_huffman.h"
using namespace std;

enum algorithm { BASIC, ADAPTIVE };

struct 
{
    char *input_fname = NULL;
    char *output_fname = NULL;
    algorithm algo = BASIC;
    bool encode = true;
} opts;

inline static void usage()
{
    cout << "usage: ./huffman -i <input-file> -o <output-file> [-e|-d] [-a <basic|adaptive>]" << endl;
}

void parse_args(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "eda:i:o:")) != -1)
    {
        switch (opt)
        {
            case 'a':
                if (!strcmp(optarg, "basic"))
                    opts.algo = BASIC;
                else if (!strcmp(optarg, "adaptive"))
                    opts.algo = ADAPTIVE;
                else
                {
                    usage();
                    exit(1);
                }
                break;
            case 'd':
                opts.encode = false;
                break;
            case 'e':
                opts.encode = true;
                break;
            case 'i':
                opts.input_fname = optarg;
                break;
            case 'o':
                opts.output_fname = optarg;
                break;
            default:
                usage();
                exit(1);
                break;
        }
    }
    
    if (!opts.input_fname || !opts.output_fname)
    {
        usage();
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);
    
    ifstream input_file(opts.input_fname, ios::binary);
    ofstream output_file(opts.output_fname, ios::binary);

    if (opts.algo == BASIC)
    {
        unordered_map<alphabet, tuple<alphabet, alphabet>> node; // The tuple stands for (set, bit)
        unordered_map<alphabet, tuple<alphabet, alphabet>> set;

        count_alphabet(input_file);
        huffman(node, set);

        input_file.clear();
        input_file.seekg(0, std::ios::beg);

        fill_code_table(node, set);
        output(input_file, output_file);

        free_code_table(code_table, 0);
    } else if (opts.algo == ADAPTIVE) {
        init_coding_tree();
        adaptive_huffman(input_file, output_file);
        free_coding_tree(root);
    }

    input_file.close();
    output_file.close();

    return 0;
};