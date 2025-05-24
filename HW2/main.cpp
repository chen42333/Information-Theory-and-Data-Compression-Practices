#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <string.h>
#include <unordered_map>
using namespace std;

#define N_SYMBOLS 2
#define DEF_PPM_ORDER 2
#define ESC (char)0x1b

typedef enum prob_model { FIX, PPM } prob_model;
typedef enum coding_algo { NATURAL, UNARY } coding_algo;

struct 
{
    char *input_fname = NULL;
    char *output_fname = NULL;
    prob_model pm;
    coding_algo coding;
    int ppm_order = DEF_PPM_ORDER;
} opts;

uint64_t cnt[2] = { 0 };
double prob[2] = { 0.0 };
uint8_t output_c = 0;
int output_idx = CHAR_BIT - 1;
size_t output_cnt = 0;

inline static void usage()
{
    cerr << "usage: ./ac -i <input-file> -o <output-file> -c <natural|unary> -p <fix|ppm> [-n <PPM-order>]" << endl;
}

void parse_args(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "c:i:n:o:p:")) != -1)
    {
        switch (opt)
        {
            case 'c':
                if (!strcmp(optarg, "natural"))
                    opts.coding = NATURAL;
                else if (!strcmp(optarg, "unary"))
                    opts.coding = UNARY;
                else
                {
                    usage();
                    exit(1);
                }
                break;
            case 'i':
                opts.input_fname = optarg;
                break;
            case 'o':
                opts.output_fname = optarg;
                break;
            case 'p':
                if (!strcmp(optarg, "fix"))
                    opts.pm = FIX;
                else if (!strcmp(optarg, "ppm"))
                    opts.pm = PPM;
                else
                {
                    usage();
                    exit(1);
                }
                break;
            case 'n':
                try
                {
                    opts.ppm_order = stoi(optarg);
                }
                catch(const std::exception& e)
                {
                    cerr << e.what() << endl;
                    exit(1);
                }
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

void compute_prob(ifstream &input_file)
{
    char c;

    if (opts.coding == NATURAL) 
    {
        while (input_file.get(c))
        {
            cnt[0] += (sizeof(uint8_t) << 3) - __builtin_popcount((uint8_t)c);
            cnt[1] += __builtin_popcount((uint8_t)c);
        }
        cout << "Input file size: " << (cnt[0] + cnt[1]) / CHAR_BIT << " bytes" << endl;
    } else if (opts.coding == UNARY) {
        while (input_file.get(c))
        {
            cnt[0]++;
            cnt[1] += (uint8_t)c;
        }
        cout << "Input file size: " << cnt[0] << " bytes" << endl;
    }
    
    prob[0] = (double)cnt[0] / (double)(cnt[0] + cnt[1]);
    prob[1] = (double)cnt[1] / (double)(cnt[0] + cnt[1]);

    cout << "Prob:\n\t0: " << prob[0] << ", 1: " << prob[1] << endl;
    cout << "Input symbols (binarized): " << cnt[0] + cnt[1] << endl;
}

void scale(ofstream &output_file, double &upper_bound, double &lower_bound)
{
    while (lower_bound < 0.5 && upper_bound <= 0.5 || lower_bound >= 0.5 && upper_bound > 0.5)
    {
        if (lower_bound >= 0.5)
        {
            lower_bound -= 0.5;
            upper_bound -= 0.5;
            output_c |= 0b1;
        }

        lower_bound *= 2;
        upper_bound *= 2;

        output_c <<= 1;
        output_cnt++;
        if (output_idx-- == 0)
        {
            output_file << output_c;
            output_c = 0;
            output_idx = CHAR_BIT - 1;
        }
    }
}

// Alphabet order (low to high): '0', '1'
void encode(ifstream &input_file, ofstream &output_file, double prob[])
{
    char c;
    double upper_bound = 1.0, lower_bound = 0.0;

    while (input_file.get(c))
    {
        if (opts.coding == NATURAL)
        {
            for (uint8_t mask = 1ULL << (CHAR_BIT - 1); mask > 0; mask >>= 1)
            {
                if (c & mask)
                    lower_bound += (upper_bound - lower_bound) * prob[0];
                else
                    upper_bound -= (upper_bound - lower_bound) * prob[1];
            }
        } else if (opts.coding == UNARY) {
            for (uint8_t i = 0; i < (uint8_t)c; i++)
                lower_bound += (upper_bound - lower_bound) * prob[0];

            upper_bound -= (upper_bound - lower_bound) * prob[1];
        }

        scale(output_file, upper_bound, lower_bound);
    }

    if (output_idx != CHAR_BIT - 1)
    {
        output_c <<= output_idx + 1;
        output_file << output_c;
    }

    cout << "Output symbols: " << output_cnt << endl;
}

void _encode(string &ctx, char symbol, double &upper_bound, double &lower_bound, unordered_map<string,size_t> &ctx_total_cnt, unordered_map<string, unordered_map<char,size_t>> ctx_table[])
{
    int i;

    for (i = ctx.length(); i >= 0; i--)
    {
        unordered_map<string, unordered_map<char,size_t>> &_ctx_table = ctx_table[i];
        string _ctx = ctx.substr(ctx.length() - i, i);
        size_t cnt_esc, cnt_0, cnt_1;
        double range = upper_bound - lower_bound;

        if (_ctx_table.find(_ctx) == _ctx_table.end())
        {
            _ctx_table[_ctx][ESC] = 1;
            _ctx_table[_ctx][symbol] = 1;
            ctx_total_cnt[_ctx] = 2;
            continue;
        }

        cnt_esc = _ctx_table[_ctx][ESC];
        cnt_0 = (_ctx_table[_ctx].find('0') == _ctx_table[_ctx].end()) ? 0.0 : _ctx_table[_ctx]['0'];
        cnt_1 = (_ctx_table[_ctx].find('1') == _ctx_table[_ctx].end()) ? 0.0 : _ctx_table[_ctx]['1'];

        if (_ctx_table[_ctx].find(symbol) == _ctx_table[_ctx].end())
        {
            upper_bound -= range * (cnt_0 + cnt_1) / ctx_total_cnt[_ctx];
            _ctx_table[_ctx][symbol] = 1;
            ctx_total_cnt[_ctx]++;
            continue;
        }
        
        if (symbol == '0')
        {
            upper_bound -= range * cnt_1 / ctx_total_cnt[_ctx];
            lower_bound += range * cnt_esc / ctx_total_cnt[_ctx];
        } 
        else if (symbol == '1')
            lower_bound += range * (cnt_esc + cnt_0) / ctx_total_cnt[_ctx];
        
        _ctx_table[_ctx][symbol]++;
        ctx_total_cnt[_ctx]++;

        break;
    }

    if (i < 0)
    {
        if (symbol == '0')
            upper_bound -= 1.0 / N_SYMBOLS;
        else if (symbol == '1')
            lower_bound += 1.0 / N_SYMBOLS;
    }

    if (ctx.length() == opts.ppm_order)
        ctx.erase(0, 1);
    ctx += symbol;
}

// Alphabet order (low to high): ESC, '0', '1'
void encode(ifstream &input_file, ofstream &output_file, unordered_map<string,size_t> &ctx_total_cnt, unordered_map<string, unordered_map<char,size_t>> ctx_table[])
{
    char c;
    string ctx = "";
    double upper_bound = 1.0, lower_bound = 0.0;

    while (input_file.get(c))
    {
        if (opts.coding == NATURAL)
        {
            for (uint8_t mask = 1ULL << (CHAR_BIT - 1); mask > 0; mask >>= 1)
                _encode(ctx, (c & mask) ? '1' : '0', upper_bound, lower_bound, ctx_total_cnt, ctx_table);
        } else if (opts.coding == UNARY) {
            for (uint8_t i = 0; i < (uint8_t)c; i++)
                _encode(ctx, '1', upper_bound, lower_bound, ctx_total_cnt, ctx_table);
            _encode(ctx, '0', upper_bound, lower_bound, ctx_total_cnt, ctx_table);
        }

        scale(output_file, upper_bound, lower_bound);
    }

    if (output_idx != CHAR_BIT - 1)
    {
        output_c <<= output_idx + 1;
        output_file << output_c;
    }

    cout << "Output symbols: " << output_cnt << endl;
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);
    ifstream input_file(opts.input_fname, ios::binary);
    ofstream output_file(opts.output_fname, ios::binary);

    compute_prob(input_file); // Only for output statistics of input file for PPM

    input_file.clear();
    input_file.seekg(0, std::ios::beg);

    if (opts.pm == FIX)
        encode(input_file, output_file, prob);
    else if (opts.pm == PPM) {
        unordered_map<string,size_t> ctx_total_cnt;
        unordered_map<string, unordered_map<char,size_t>> ctx_table[opts.ppm_order + 1];

        encode(input_file, output_file, ctx_total_cnt, ctx_table);
    }

    cout << "Compression ratio: " << (double)(cnt[0] + cnt[1] - output_cnt) / (cnt[0] + cnt[1]) * 100 << "%" << endl;

    return 0;
}