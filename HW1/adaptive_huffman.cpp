#include "adaptive_huffman.h"

#define swap(a, b) (a ^= b, b ^= a, a ^= b)

coding_tree_node *root;
uint64_t next_id;
unordered_map<uint64_t, set<uint64_t>> blocks; // weight to block
unordered_map<uint64_t, coding_tree_node*> node_table; // id to node
unordered_map<alphabet, uint64_t> id_table; // alphabet to id
coding_tree_node *NYT;
uint64_t num_input = 0, num_output = 0;

char output_c = 0;
int output_c_remain = 0;

void init_coding_tree()
{
    uint64_t root_id = (1ULL << (BITS + 1)) - 1;

    root = (coding_tree_node*)calloc(1, sizeof(coding_tree_node));
    *root = {.id = root_id, .weight = 0, .has_alphabet = false};
    NYT = root;
    blocks[0].insert(root_id);
    next_id = root_id - 1;
}

void free_coding_tree(coding_tree_node *nd)
{
    if (nd->children[0])
        free_coding_tree(nd->children[0]);
    if (nd->children[1])
        free_coding_tree(nd->children[1]);

    free(nd);
}

inline static void flush_output_buffer(ofstream &output_file)
{
    if (output_c_remain)
        output_file.write(&output_c, 1);
}

static void output_code(alphabet c, bool is_first, ofstream &output_file)
{
    coding_tree_node *nd;
    stack<uint8_t> path;

    if (is_first)
        nd = NYT;
    else
        nd = node_table[id_table[c]];

    while (nd != root)
    {
        if (nd->parent->children[0] == nd)
            path.push(0);
        else
            path.push(1);
        
        nd = nd->parent;
    }

    while (!path.empty())
    {
        output_c |= (path.top() << (CHAR_BIT - output_c_remain - 1));
        if (++output_c_remain == CHAR_BIT)
        {
            output_file.write(&output_c, 1);
            output_c = 0;
            output_c_remain = 0;
            num_output++;
        }
        path.pop();
    }

    if (is_first)
    {
        for (int i = 0; i < sizeof(alphabet); i++)
        {
            uint8_t byte = ((uint8_t*)&c)[i];
            output_c |= byte >> output_c_remain;
            output_file.write(&output_c, 1);
            output_c = byte << (CHAR_BIT - output_c_remain);
            num_output++;
        }
    }
}

static void update_coding_tree(alphabet c, bool is_first)
{
    coding_tree_node *nd;

    if (is_first)
    {
        NYT->children[1] = (coding_tree_node*)calloc(1, sizeof(coding_tree_node));
        *NYT->children[1] = {.id = next_id--, .weight = 0, .has_alphabet = true, .c = c, .parent = NYT};
        NYT->children[0] = (coding_tree_node*)calloc(1, sizeof(coding_tree_node));
        *NYT->children[0] = {.id = next_id--, .weight = 0, .has_alphabet = false, .parent = NYT};

        // Update blocks
        blocks[0].erase(NYT->id);
        NYT->weight++;
        blocks[1].insert(NYT->id);

        blocks[0].insert(NYT->children[0]->id);

        NYT->children[1]->weight++;
        blocks[1].insert(NYT->children[1]->id);

        // Update node_table and NYT
        node_table[NYT->children[0]->id] = NYT->children[0];
        node_table[NYT->children[1]->id] = NYT->children[1];
        id_table[c] = NYT->children[1]->id;
        nd = NYT;
        NYT = NYT->children[0];

        if (nd == root)
            return;
        nd = nd->parent;
    }
    else
        nd = node_table[id_table[c]];

    while (true) 
    {
        set<uint64_t> &block = blocks[nd->weight];
        uint64_t max_id = *(block.rbegin());
        uint64_t max_id_2 = (block.size() > 1) ? *(next(block.rbegin(), 1)) : 0;

        if (nd->id != max_id && !(nd->parent->id == max_id && nd->id == max_id_2)) // If not the max id of the block (excluding its direct parent)
        {
            coding_tree_node *target_nd = node_table[max_id];

            // Update parent
            if (nd->parent->children[0] == nd)
                nd->parent->children[0] = target_nd;
            else 
                nd->parent->children[1] = target_nd;

            if (target_nd->parent->children[0] == target_nd)
                target_nd->parent->children[0] = nd;
            else 
                target_nd->parent->children[1] = nd;

            swap(*(uint64_t*)&nd->parent, *(uint64_t*)&target_nd->parent);

            // Update node id
            swap(nd->id, target_nd->id);

            // Update node table
            node_table[nd->id] = nd;
            node_table[target_nd->id] = target_nd;

            // Update id table
            if (nd->has_alphabet)
                id_table[nd->c] = nd->id;
            if (target_nd->has_alphabet)
                id_table[target_nd->c] = target_nd->id;
        }

        // Update blocks
        block.erase(nd->id);
        blocks[++nd->weight].insert(nd->id);

        if (nd == root)
            break;
        nd = nd->parent;
    }
}

void adaptive_huffman(ifstream &input_file, ofstream &output_file)
{
    alphabet c;

    while (input_file.read(reinterpret_cast<char*>(&c), sizeof(alphabet)) || input_file.gcount() > 0)
    {
        bool is_first;

        if (input_file.gcount() != sizeof(alphabet))
        {
            alphabet mask = (1ULL << (input_file.gcount() * CHAR_BIT)) - 1;   
            c &= mask;
        }

        num_input++;

        is_first = (id_table.find(c) == id_table.end());
        output_code(c, is_first, output_file);
        update_coding_tree(c, is_first);
    }

    flush_output_buffer(output_file);

    cout << "Average codeword length: " << (double)num_output / num_input * CHAR_BIT << endl;
}

inline static bool is_external_node(coding_tree_node *node)
{
    return !node->children[0] && !node->children[1];
}

void adaptive_huffman_decode(ifstream &input_file, ofstream &output_file)
{
    uint8_t c = 0;
    int c_remain = 0;

    while (true)
    {
        coding_tree_node *nd = root;

        while (!is_external_node(nd))
        {
            if (!c_remain)
            {
                if (!input_file.read(reinterpret_cast<char*>(&c), 1))
                    return;
                c_remain = CHAR_BIT;
            }
                
            nd = nd->children[c >> (CHAR_BIT - 1)];
            c <<= 1;
            c_remain--;
        }

        if (nd != NYT)
        {
            update_coding_tree(nd->c, false);
            output_file.write(reinterpret_cast<char*>(&nd->c), sizeof(alphabet));
        } else {
            alphabet new_c = 0;

            if (!c_remain)
            {
                if (!input_file.read(reinterpret_cast<char*>(&new_c), sizeof(alphabet)) || input_file.gcount() != sizeof(alphabet))
                    return;
            } else {
                alphabet buf;
                
                if (!input_file.read(reinterpret_cast<char*>(&buf), sizeof(alphabet)) || input_file.gcount() != sizeof(alphabet))
                    return;

                for (int i = 0; i < sizeof(alphabet); i++)
                {
                    if (!i)
                        ((uint8_t*)&new_c)[i] = c;

                    ((uint8_t*)&new_c)[i] |= ((uint8_t*)&buf)[i] >> c_remain;
                    
                    if (i < sizeof(alphabet) - 1)
                        ((uint8_t*)&new_c)[i + 1] |= ((uint8_t*)&buf)[i] << (CHAR_BIT - c_remain);
                    else
                        c = ((uint8_t*)&buf)[i] << (CHAR_BIT - c_remain);
                }
            }
            update_coding_tree(new_c, true);
            output_file.write(reinterpret_cast<char*>(&new_c), sizeof(alphabet));
        }
    }
}