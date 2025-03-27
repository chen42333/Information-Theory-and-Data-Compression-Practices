#include <iostream>
#include <fstream>
#include <map>
#include <tuple>
#include <queue>
#include <stack>
using namespace std;

#define BITS 8
#define TABLE_SIZE_EXP 8

typedef
#if (BITS == 64)
uint64_t 
#elif (BITS == 32)
uint32_t
#elif (BITS == 16)
uint16_t
#elif (BITS == 8)
uint8_t
#endif
alphabet;

#define INIT_VAL (alphabet)0xff

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

int num_alphabet = 0;
void *count_table[1ULL << TABLE_SIZE_EXP] = { NULL };
int page_level = (sizeof(alphabet) << 3) / TABLE_SIZE_EXP;

inline static void usage()
{
    cout << "usage: ./huffman <input-file> <output-file>" << endl;
}

void count_alphabet(ifstream &file)
{
    alphabet c;

    while (file.read(reinterpret_cast<char*>(&c), sizeof(alphabet)) || file.gcount() > 0)
    {
        void **cur_table = count_table;

        if (file.gcount() != sizeof(alphabet))
        {
            alphabet mask = (1ULL << (file.gcount() << 3)) - 1;   
            c &= mask;
        }

        num_alphabet++;

        for (int i = 0; i < page_level; i++)
        {
            int idx = ((uint8_t*)&c)[i];

            if (i == page_level - 1)
            {
                if (!cur_table[idx])
                    cur_table[idx] = calloc(1, sizeof(uint64_t));
                (*(uint64_t*)cur_table[idx])++;
            } else {
                if (!cur_table[idx])
                    cur_table[idx] = calloc(1, sizeof(void*) * (1ULL << TABLE_SIZE_EXP));
                cur_table = (void**)cur_table[idx];
            }
        }
    }
}

void compute_prob(void **table, priority_queue<prob_node> &prob, map<alphabet, tuple<alphabet, alphabet>> &node, int level, alphabet prev_idx)
{
    if (level == page_level)
    {
        double p = *(uint64_t*)table / (double)num_alphabet;
        prob.push({prev_idx, p, false});
        node[prev_idx] = make_tuple(prev_idx, INIT_VAL);
        free(table);
        return;
    }

    for (alphabet i = 0; ; i++)
    {
        if (table[i])
            compute_prob((void**)table[i], prob, node, level + 1, (prev_idx << TABLE_SIZE_EXP) | i);
        
        if (i == (1ULL << TABLE_SIZE_EXP) - 1) // Prevent the infinite loop due to overflow of i when BITS == 8
            break;
    }
        
    
    if (table != count_table)
        free(table);
}

void huffman(map<alphabet, tuple<alphabet, alphabet>> &node, map<alphabet, tuple<alphabet, alphabet>> &set)
{
    priority_queue<prob_node> prob;
    int num_kind_alphabet;

    compute_prob(count_table, prob, node, 0, 0);
    num_kind_alphabet = prob.size();

    for (alphabet i = 0; i < num_kind_alphabet - 1; i++)
    {
        prob_node least_prob[2];
        double new_prob;

        for (int j = 0; j < 2; j++)
        {
            least_prob[j] = prob.top();
            prob.pop();

            if (least_prob[j].is_set)
                set[least_prob[j].c] = make_tuple(i, j);
            else
                node[least_prob[j].c] = make_tuple(i, j);
        }

        set[i] = make_tuple(i, INIT_VAL);

        new_prob = least_prob[0].p + least_prob[1].p;
        prob.push({i, new_prob, true});
    }
}

void output(ifstream &input_file, ofstream &output_file, map<alphabet, tuple<alphabet, alphabet>> &node, map<alphabet, tuple<alphabet, alphabet>> &set)
{
    alphabet input_c;
    char output_c = 0;
    int output_c_idx = 7;

    while (input_file.read(reinterpret_cast<char*>(&input_c), sizeof(alphabet)) || input_file.gcount() > 0)
    {
        if (input_file.gcount() != sizeof(alphabet))
        {
            alphabet mask = (1ULL << (input_file.gcount() << 3)) - 1;   
            input_c &= mask;
        }

        stack<alphabet> codeword_bit;
        alphabet cur_set = get<0>(node[input_c]);
        codeword_bit.push(get<1>(node[input_c]));

        while (get<0>(set[cur_set]) != cur_set) // or get<1>(set[cur_set]) != INIT_VAL
        {
            codeword_bit.push(get<1>(set[cur_set]));
            cur_set = get<0>(set[cur_set]);
        }

        while (!codeword_bit.empty())
        {
            output_c |= (codeword_bit.top() << output_c_idx);
            if (!output_c_idx--)
            {
                output_file.write(&output_c, 1);
                output_c = 0;
                output_c_idx = 7;
            }
            codeword_bit.pop();
        }
    }

    if (output_c_idx < 7)
        output_file.write(&output_c, 1);
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
    map<alphabet, tuple<alphabet, alphabet>> node; // The tuple stands for (set, bit)
    map<alphabet, tuple<alphabet, alphabet>> set;

    count_alphabet(input_file);
    huffman(node, set);

    input_file.clear();
    input_file.seekg(0, std::ios::beg);

    output(input_file, output_file, node, set);

    input_file.close();
    output_file.close();

    return 0;
};