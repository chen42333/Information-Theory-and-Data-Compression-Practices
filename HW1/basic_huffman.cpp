#include "basic_huffman.h"

#define INIT_VAL (alphabet)0xff

int num_alphabet = 0;
void *count_table[1ULL << TABLE_SIZE_EXP] = { NULL };
void *code_table[1ULL << TABLE_SIZE_EXP] = { NULL };
int page_level = sizeof(alphabet) * CHAR_BIT / TABLE_SIZE_EXP;

void count_alphabet(ifstream &file)
{
    alphabet c;

    while (file.read(reinterpret_cast<char*>(&c), sizeof(alphabet)) || file.gcount() > 0)
    {
        void **cur_table = count_table;

        if (file.gcount() != sizeof(alphabet))
        {
            alphabet mask = (1ULL << (file.gcount() * CHAR_BIT)) - 1;   
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
                if (!cur_table[idx])
                {
                    cout << "Error: calloc() failed" << endl;
                    exit(1);
                }
                (*(uint64_t*)cur_table[idx])++;
            } else {
                if (!cur_table[idx])
                    cur_table[idx] = calloc(1, sizeof(void*) * (1ULL << TABLE_SIZE_EXP));
                if (!cur_table[idx])
                {
                    cout << "Error: calloc() failed" << endl;
                    exit(1);
                }
                cur_table = (void**)cur_table[idx];
            }
        }
    }
}

static void compute_prob(void **table, priority_queue<prob_node> &prob, map<alphabet, tuple<alphabet, alphabet>> &node, int level, alphabet idx)
{
    if (level == page_level)
    {
        double p = *(uint64_t*)table / (double)num_alphabet;
        prob.push({.c = idx, .p = p, .is_set = false});
        node[idx] = make_tuple(idx, INIT_VAL);
        free(table);
        return;
    }

    for (alphabet i = 0; ; i++)
    {
        if (table[i])
        {
            ((uint8_t*)&idx)[level] = i;
            compute_prob((void**)table[i], prob, node, level + 1, idx);
        }
            
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
        prob.push({.c = i, .p = new_prob, .is_set = true});
    }
}

void free_code_table(void **table, int level)
{
    if (level == page_level)
    {
        free(table);
        return;
    }

    for (alphabet i = 0; ; i++)
    {
        if (table[i])
            free_code_table((void**)table[i], level + 1);
        
        if (i == (1ULL << TABLE_SIZE_EXP) - 1) // Prevent the infinite loop due to overflow of i when BITS == 8
            break;
    }
        
    if (table != code_table)
        free(table);
}

void fill_code_table(map<alphabet, tuple<alphabet, alphabet>> &node, map<alphabet, tuple<alphabet, alphabet>> &set)
{
    for (const auto &_node: node)
    {
        stack<alphabet> codeword_bit;
        code_node *nd;
        uint8_t *c_ptr;
        int c_idx = CHAR_BIT - 1;
        void **cur_table = code_table;
        alphabet cur_set = get<0>(_node.second);
        codeword_bit.push(get<1>(_node.second));

        while (get<0>(set[cur_set]) != cur_set) // or get<1>(set[cur_set]) != INIT_VAL
        {
            codeword_bit.push(get<1>(set[cur_set]));
            cur_set = get<0>(set[cur_set]);
        }

        nd = (code_node*)calloc(1, sizeof(uint64_t) + (codeword_bit.size() + CHAR_BIT - 1) / CHAR_BIT);
        if (!nd)
        {
            cout << "Error: calloc() failed" << endl;
            exit(1);
        }
        nd->len = codeword_bit.size();
        c_ptr = nd->code;

        while (!codeword_bit.empty())
        {
            *c_ptr |= (codeword_bit.top() << c_idx);
            if (!c_idx--)
            {
                c_ptr++;
                c_idx = CHAR_BIT - 1;
            }
            codeword_bit.pop();
        }

        for (int i = 0; i < page_level; i++)
        {
            int idx = ((uint8_t*)&_node.first)[i];

            if (i == page_level - 1)
                cur_table[idx] = nd;
            else 
            {
                if (!cur_table[idx])
                    cur_table[idx] = calloc(1, sizeof(void*) * (1ULL << TABLE_SIZE_EXP));
                if (!cur_table[idx])
                {
                    cout << "Error: calloc() failed" << endl;
                    exit(1);
                }
                cur_table = (void**)cur_table[idx];
            }
        }
    }
}

void output(ifstream &input_file, ofstream &output_file)
{
    alphabet input_c;
    char output_c = 0;
    int output_c_remain = 0;

    while (input_file.read(reinterpret_cast<char*>(&input_c), sizeof(alphabet)) || input_file.gcount() > 0)
    {
        void **cur_table = code_table;
        code_node *nd;
        int remain_bits;

        if (input_file.gcount() != sizeof(alphabet))
        {
            alphabet mask = (1ULL << (input_file.gcount() * CHAR_BIT)) - 1;   
            input_c &= mask;
        }

        for (int i = 0; i < page_level; i++)
        {
            int idx = ((uint8_t*)&input_c)[i];

            if (!cur_table[idx])
            {
                cout << "Error: code not found" << endl;
                exit(1);
            }
            cur_table = (void**)cur_table[idx];
        }

        nd = (code_node*)cur_table;
        remain_bits = nd->len % CHAR_BIT;

        for (int i = 0; i < nd->len / CHAR_BIT; i++)
        {
            output_c |= nd->code[i] >> output_c_remain;
            output_file.write(&output_c, 1);
            output_c = nd->code[i] << (CHAR_BIT - output_c_remain);
        }

        if (remain_bits)
        {
            output_c |= nd->code[nd->len / CHAR_BIT] >> output_c_remain;
            if (output_c_remain + remain_bits >= CHAR_BIT)
            {
                output_file.write(&output_c, 1);
                output_c = nd->code[nd->len / CHAR_BIT] << (CHAR_BIT - output_c_remain);
                output_c_remain = output_c_remain + remain_bits - CHAR_BIT;
            }
            else
                output_c_remain += remain_bits;
        }
    }

    if (output_c_remain)
        output_file.write(&output_c, 1);
}