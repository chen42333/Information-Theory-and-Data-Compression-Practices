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
    char *pmf_fname = NULL;
} opts;

inline static void usage()
{
    cerr << "usage: ./huffman -i <input-file> -o <output-file> [-e|-d] [-a <basic|adaptive>] [-p]" << endl;
}

void parse_args(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "eda:i:o:p:")) != -1)
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
            case 'p':
                opts.pmf_fname = optarg;
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
    if (opts.pmf_fname && (!opts.encode || opts.algo != BASIC))
        cerr << "Warning: generate PMF information in only supported for basic huffman encoding" << endl;
    
    ifstream input_file(opts.input_fname, ios::binary);
    ofstream output_file(opts.output_fname, ios::binary);

    if (opts.algo == BASIC)
    {
        if (opts.encode)
        {
            unordered_map<alphabet, tuple<alphabet, alphabet, double>> node; // The tuple stands for (set, bit, prob)
            unordered_map<alphabet, tuple<alphabet, alphabet>> set;

            count_alphabet(input_file);

            if (opts.pmf_fname)
            {
                ofstream pmf_file(opts.pmf_fname);

                input_file.clear();
                input_file.seekg(0, std::ios::beg);

                output_pmf_info(input_file, pmf_file);

                pmf_file.close();
            }

            huffman(node, set);

            input_file.clear();
            input_file.seekg(0, std::ios::beg);

            fill_code_table(node, set);
            output(input_file, output_file);
            free_code_table(code_table, 0);
        } else {
            fill_code_table_decode(input_file);
            huffman_decode(input_file, output_file);
            free_code_table_decode(code_table);
        }
    } else if (opts.algo == ADAPTIVE) {
        init_coding_tree();
        if (opts.encode)
            adaptive_huffman(input_file, output_file);
        else 
            adaptive_huffman_decode(input_file, output_file);
        free_coding_tree(root);
    }

    input_file.close();
    output_file.close();

    return 0;
};