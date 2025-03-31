#include "basic_huffman.h"

#define INIT_VAL (alphabet)0xff

int num_alphabet = 0;
void *count_table[1ULL << TABLE_SIZE_EXP] = { NULL };
void *code_table[1ULL << TABLE_SIZE_EXP] = { NULL };
int page_level = sizeof(alphabet) * CHAR_BIT / TABLE_SIZE_EXP;
uint64_t code_table_size = 0;
uint8_t num_bytes = 0; // Number of bytes of the original file (mod 256)

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

        num_bytes += file.gcount();
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

static void compute_prob(void **table, priority_queue<prob_node> &prob, unordered_map<alphabet, tuple<alphabet, alphabet>> &node, int level, alphabet idx)
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

void huffman(unordered_map<alphabet, tuple<alphabet, alphabet>> &node, unordered_map<alphabet, tuple<alphabet, alphabet>> &set)
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

void fill_code_table(unordered_map<alphabet, tuple<alphabet, alphabet>> &node, unordered_map<alphabet, tuple<alphabet, alphabet>> &set)
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
        code_table_size += sizeof(alphabet) + sizeof(uint64_t) + (codeword_bit.size() + CHAR_BIT - 1) / CHAR_BIT;

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

static void output_code_table(ofstream &output_file, void **table, int level, alphabet idx)
{
    if (level == page_level)
    {
        code_node *nd = (code_node*)table;
        output_file.write(reinterpret_cast<char*>(&idx), sizeof(alphabet));
        output_file.write(reinterpret_cast<char*>(&nd->len), sizeof(nd->len));
        output_file.write(reinterpret_cast<char*>(nd->code), (nd->len + CHAR_BIT - 1) / CHAR_BIT);
        return;
    }

    for (alphabet i = 0; ; i++)
    {
        ((uint8_t*)&idx)[level] = i;

        if (table[i])
            output_code_table(output_file, (void**)table[i], level + 1, idx);
        
        if (i == (1ULL << TABLE_SIZE_EXP) - 1) // Prevent the infinite loop due to overflow of i when BITS == 8
            break;
    }
}

void output(ifstream &input_file, ofstream &output_file)
{
    alphabet input_c;
    char output_c = 0;
    int output_c_remain = 0;

    output_file.write(reinterpret_cast<char*>(&num_bytes), sizeof(num_bytes));
    output_file.write(reinterpret_cast<char*>(&code_table_size), sizeof(code_table_size));
    output_code_table(output_file, code_table, 0, 0);

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

const uint64_t one_level_table_size = sizeof(void*) * (1ULL << TABLE_SIZE_EXP);

void free_code_table_decode(void **table)
{
    for (alphabet i = 0; ; i++)
    {
        if (malloc_size(table[i]) >= one_level_table_size)
            free_code_table_decode((void**)table[i]);
        else if (table[i] && !(i != 0 && table[i] == table[i - 1]))
            free(table[i]);
        
        if (i == (1ULL << TABLE_SIZE_EXP) - 1) // Prevent the infinite loop due to overflow of i when BITS == 8
            break;
    }
        
    if (table != code_table)
        free(table);
}

static void __fill_code_table_decode(void **table)
{
    for (alphabet i = 0; ; i++)
    {
        if (!table[i])
        {
            if (i == 0)
            {
                cout << "Error: the first entry of table is empty" << endl;
                exit(1);
            }
            else
                table[i] = table[i - 1];
        }
        else if (malloc_size(table[i]) >= one_level_table_size)
            __fill_code_table_decode((void**)table[i]);

        if (i == (1ULL << TABLE_SIZE_EXP) - 1) // Prevent the infinite loop due to overflow of i when BITS == 8
            break;
    }
}

void fill_code_table_decode(ifstream &input_file)
{
    uint64_t read_size = 0;

    if (!input_file.read(reinterpret_cast<char*>(&num_bytes), sizeof(num_bytes))
    || !input_file.read(reinterpret_cast<char*>(&code_table_size), sizeof(uint64_t)))
    {
        cout << "Read error" << endl;
        exit(1);
    }

    while (read_size < code_table_size)
    {
        code_table_node *nd = (code_table_node*)calloc(1, sizeof(code_table_node));
        void **ptr = code_table;
        uint64_t num_of_bytes;

        input_file.read(reinterpret_cast<char*>(&nd->c), sizeof(alphabet));
        input_file.read(reinterpret_cast<char*>(&nd->len), sizeof(uint64_t));
        num_of_bytes = (nd->len + CHAR_BIT - 1) / CHAR_BIT;

        for (int i = 0; i < num_of_bytes; i++)
        {
            uint8_t byte;

            input_file.read(reinterpret_cast<char*>(&byte), 1);

            if (i == num_of_bytes - 1)
                ptr[byte] = nd;
            else
            {
                if (!ptr[byte])
                    ptr[byte] = calloc(1, sizeof(void*) * (1ULL << TABLE_SIZE_EXP));
                ptr = (void**)ptr[byte];
            }
        }

        read_size += sizeof(alphabet) + sizeof(uint64_t) + num_of_bytes;
    }

    __fill_code_table_decode(code_table);
}

void huffman_decode(ifstream &input_file, ofstream &output_file)
{
    uint8_t c = 0, buf, num_writen_bytes = 0;
    int c_remain = 0;
    bool skip_read = false;

    while (true)
    {
        void **ptr = code_table;
        code_table_node *nd;
        int used_bits;

        if (!skip_read)
            input_file.read(reinterpret_cast<char*>(&buf), 1);

        while (true)
        {
            c |= buf >> c_remain;
            if (!(ptr = (void**)ptr[c]))
            {
                cout << "Error: empty code table entry" << endl;
                exit(1);
            }
            if (malloc_size(ptr) < one_level_table_size) // Node found
                break;
            c = buf << (CHAR_BIT - c_remain);
            input_file.read(reinterpret_cast<char*>(&buf), 1);
        }
        
        nd = (code_table_node*)ptr;
        used_bits = nd->len % CHAR_BIT;
        if (used_bits == 0)
            used_bits = CHAR_BIT;
        if (input_file.peek() == EOF && num_writen_bytes + sizeof(alphabet) >= num_bytes)
        {
            output_file.write(reinterpret_cast<char*>(&nd->c), num_bytes - num_writen_bytes);
            return;
        } else {
            output_file.write(reinterpret_cast<char*>(&nd->c), sizeof(alphabet));
            num_writen_bytes += sizeof(alphabet);
        }

        if (used_bits <= c_remain) // The content in buf isn't used actually
        {
            c <<= used_bits;
            c_remain = c_remain - used_bits;
            skip_read = true;
        } else {
            int shift_bits = used_bits - c_remain;

            c = buf << shift_bits;
            c_remain = CHAR_BIT - shift_bits;
            skip_read = false;
        }
    }
}