#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <queue>

using namespace std;

struct TreeNode{
    size_t frequency;
    int symbol;
    TreeNode *left;
    TreeNode *right;
};

struct TableRow {
    char symbol;
    unsigned char len;
    bool* code;
};

class TreeNodeHeap{
public:
    TreeNodeHeap() {
        for (int i = 0; i < 257; ++i) {
            data[i] = nullptr;
        }
    }

    void push(TreeNode* elem) {
        assert(_size < 256);
        data[_size] = elem;
        size_t current_index = _size++;
        auto current_value = data[current_index];
        auto parent_value = data[get_parent_index(current_index)];

        while (current_value->frequency < parent_value->frequency) {
            data[get_parent_index(current_index)] = current_value;
            data[current_index] = parent_value;
            current_index = get_parent_index(current_index);

            current_value = data[current_index];
            parent_value = data[get_parent_index(current_index)];
        }
    }

    TreeNode* pop() {
        assert(_size > 0);
        auto res = data[0];
        data[0] = data[--_size];
        data[_size] = nullptr;
        size_t current_index = 0;
        auto current_value = data[current_index];
        auto left_value = data[get_left_child_index(current_index)];
        auto right_value = data[get_right_child_index(current_index)];

        if (left_value == nullptr) {
            return res;
        } else if (right_value == nullptr) {
            if (left_value->frequency < current_value->frequency) {
                data[current_index] = left_value;
                data[get_left_child_index(current_index)] = current_value;
            }
            return res;
        }

        while (
                left_value->frequency < current_value->frequency ||
                right_value->frequency < current_value->frequency
        ) {
            if (left_value->frequency < right_value->frequency) {
                data[current_index] = left_value;
                data[get_left_child_index(current_index)] = current_value;
                current_index = get_left_child_index(current_index);
            } else {
                data[current_index] = right_value;
                data[get_right_child_index(current_index)] = current_value;
                current_index = get_right_child_index(current_index);
            }

            current_value = data[current_index];
            left_value = get_left_child_index(current_index) < 256 ? data[get_left_child_index(current_index)] : nullptr;
            right_value = get_right_child_index(current_index) < 256 ? data[get_right_child_index(current_index)] : nullptr;

            if (left_value == nullptr) {
                break;
            } else if (right_value == nullptr) {
                if (left_value->frequency < current_value->frequency) {
                    data[current_index] = left_value;
                    data[get_left_child_index(current_index)] = current_value;
                }
                break;
            }
        }
        return res;
    }

    size_t size() const {
        return _size;
    }

private:
    size_t get_left_child_index(size_t index) const {
        return 2 * index  + 1;
    }

    size_t get_right_child_index(size_t index) const {
        return 2 * index  + 2;
    }

    size_t get_parent_index(size_t index) const {
        return floor((index-1)/2);
    }

    size_t _size = 0;
    TreeNode* data[257];
};

size_t* get_frequencies(const char* data, size_t size) {
    auto frequencies = new size_t[256];
    memset(frequencies, 0, 256 * sizeof(size_t));

    for (size_t i = 0; i < size; ++i) {
        frequencies[(unsigned char)(data[i])]++;
    }

    return frequencies;
}

void set_bit_value(char *output_data, size_t offset, bool value) {
    output_data[offset / 8] = output_data[offset / 8] | (((value ? 1 : 0) << 7) >> offset % 8);
}

bool get_bit_value(const char* input_data, size_t offset) {
    return (bool)((((input_data[offset / 8] << (offset % 8))) >> 7) % 2);
}

void build_table(TreeNode* current_node, TableRow* table, bool* stack, size_t stack_size) {
    if(current_node == nullptr) {
        return;
    }

    if (current_node->symbol < 256) {
        auto row = &(table[(unsigned char)(current_node->symbol)]);
        row->symbol = char(current_node->symbol);
        row->len = stack_size;
        row->code = new bool[stack_size];
        memcpy(row->code, stack, stack_size);
    } else {
        stack[stack_size] = false;
        build_table(current_node->left, table, stack, stack_size + 1);

        stack[stack_size] = true;
        build_table(current_node->right, table, stack, stack_size + 1);
    }
}

TreeNode* build_encode_tree(size_t* frequencies) {
    TreeNodeHeap freq_heap;
    for (int i = 0; i < 256; ++i) {
        if(frequencies[i] != 0){
            freq_heap.push(new TreeNode({frequencies[i], i, nullptr, nullptr}));
        }
    }

    while (freq_heap.size() > 1) {
        TreeNode* first = freq_heap.pop();
        TreeNode* second = freq_heap.pop();
        freq_heap.push(new TreeNode({first->frequency + second->frequency, 256, first, second}));
    }
    return freq_heap.pop();
}

TreeNode* recreate_encode_tree(TableRow* table) {
    auto root = new TreeNode({0, 256, nullptr, nullptr});
    for (int i = 0; i < 256; ++i) {
        TreeNode* current_node = root;
        if (table[i].len) {
            for (int j = 0; j < table[i].len; ++j) {
                if (table[i].code[j]) { // right
                    if (current_node->right == nullptr) {
                        current_node->right = new TreeNode({0, 256, nullptr, nullptr});
                    }
                    current_node = current_node->right;
                } else { // left
                    if (current_node->left == nullptr) {
                        current_node->left = new TreeNode({0, 256, nullptr, nullptr});
                    }
                    current_node = current_node->left;
                }
            }
            current_node->symbol = (unsigned char) table[i].symbol;
        }
    }
    return root;
}

size_t res_length(size_t* frequencies, TableRow* table){
    size_t len = sizeof(size_t) * 8 + 8;
    for(int i = 0; i < 256; ++i) {
        if (table[i].len > 0) {
            len += 8 + 8 + 8 * (table[i].len / 8) + (table[i].len % 8 ? 8 : 0);
            len += frequencies[i] * table[i].len;
        }
    }
    return len;
}

void encode_with_table(char* output_data, TableRow* table, const char* data, size_t size, size_t offset) {
    for (size_t i = 0; i < size; ++i) {
        for(int j = 0; j < table[(unsigned char)(data[i])].len; ++j) {
            set_bit_value(output_data, offset, table[(unsigned char)(data[i])].code[j]);
            offset++;
        }
    }
}

size_t read_table(const char *input_data, TableRow* table, size_t offset) {
    int table_len = ((unsigned char*)(input_data))[offset / 8];
    offset += 8;

    for(int i = 0; i < table_len; ++i){
        char symbol = input_data[offset / 8];
        unsigned char table_pos = ((unsigned char*)(input_data))[offset / 8];
        offset += 8;
        unsigned char len = ((unsigned char*)(input_data))[offset / 8];
        offset += 8;
        table[table_pos].symbol = symbol;
        table[table_pos].len = len;
        table[table_pos].code = new bool[len];
        for (int j = 0; j < len; ++j) {
            table[table_pos].code[j] = get_bit_value(input_data, offset);
            offset++;
        }
        if(table[table_pos].len % 8 != 0) {
            offset += 8 - table[table_pos].len % 8;
        }
    }
    return offset;
}

size_t write_table(char *output_data, TableRow* table, size_t offset) {
    int table_len = 0;
    size_t initial_offset = offset;
    offset += 8; //will set the length of the table here
    for (int i = 0; i < 256; ++i) {
        if(table[i].len > 0) {
            table_len++;
            output_data[offset / 8] = table[i].symbol;
            offset += 8;
            ((unsigned char*)output_data)[offset / 8] = table[i].len;
            offset += 8;
            for(int j = 0; j < table[i].len; ++j) {
                set_bit_value(output_data, offset, table[i].code[j]);
                offset++;
            }
            if(table[i].len % 8 != 0){
                offset += 8 - table[i].len % 8;
            }
        }
    }

    ((unsigned char*)output_data)[initial_offset / 8] = (unsigned char)table_len;

    return offset;
}

void print_table(TableRow* table) {
    for (int i = 0; i < 256; ++i) {
        if (table[i].len > 0) {
            printf("%c: ", table[i].symbol);
            for (int j = 0; j < table[i].len; ++j) {
                printf("%d", table[i].code[j] ? 1 : 0);
            }
            printf("\n");
        }
    }
}

char* encode(const char* data, size_t &size) {

    auto frequencies = get_frequencies(data, size);
    auto root = build_encode_tree(frequencies);


    auto table = new TableRow[256];
    auto stack = new bool[256];

    build_table(root, table, stack, 0);

    size_t max_offset = res_length(frequencies, table); //in bits
    size_t output_len = (max_offset / 8) + (max_offset % 8 ? 1 : 0); //in bytes

    char* output_data = new char[output_len];
    memset(output_data, 0, output_len);

    *((size_t*)(void*)output_data) = max_offset;

    size_t offset = write_table(output_data, table, 64);
    encode_with_table(output_data, table, data, size, offset);
    size = output_len;

    print_table(table);

    delete[] frequencies;
    return output_data;
}

char *recreate_data(const char *input_data, TreeNode* root, size_t offset, size_t max_offset, size_t &size) {
    char *recreated_data = (char *)calloc(128, sizeof(char)); //will realloc if needed
    size_t allocated_size = 128;
    size = 0;

    TreeNode* current_node = root;
    for (; offset < max_offset; ++offset) {
        bool value = get_bit_value(input_data, offset);
        current_node = value ? current_node->right : current_node->left;
        if (current_node->symbol < 256) {
            recreated_data[size] = char(current_node->symbol);

            size++;
            if (size == allocated_size) {
                realloc(recreated_data, allocated_size * 2);
                allocated_size *= 2;
            }

            current_node = root;
        }
    }
    return recreated_data;
}

char* decode(const char *input_data, size_t& size){
    auto table = new TableRow[256];
    for (int i = 0; i < 256; ++i) {
        table[i] = TableRow({0, 0, nullptr});
    }
    size_t max_offset = *((size_t*)(void*)input_data);
    size_t offset = read_table(input_data, table, 64);

    TreeNode* root = recreate_encode_tree(table);
    char* decoded = recreate_data(input_data, root, offset, max_offset, size);

    return decoded;
}

char* read_file(const char* filename, size_t& file_size) {
    FILE *file;
    char *buffer;

    file = fopen (filename, "rb" );
    if( !file ) {
        perror(filename);
        exit(1);
    }

    fseek(file, 0l, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    buffer = (char*)(calloc( 1, file_size+1 ));
    if( !buffer ) {
        printf("memory alloc fails");
        exit(1);
    }

    if( 1!=fread( buffer , file_size, 1 , file) ) {
        printf("entire read fails");
        exit(1);
    }

    fclose(file);
    return buffer;
}

void write_file(const char* filename, const char* data, size_t size) {
    FILE *file = fopen(filename, "wb");

    int results = fwrite((void*)data, size, 1, file);
    if (results == EOF) {
        printf("Can't write to file %s\n", filename);
    }

    fclose(file);
}

int main(int argc, char **argv) {

    assert(argc >= 3);
    char* mode = argv[1];
    size_t size;
    char* input = read_file(argv[2], size);


    if(strcmp(mode, "encode") == 0) {
        char* output = encode(input, size);
        string filename = argv[2];
        write_file((filename + ".archive").c_str(), output, size);
    } else if(strcmp(mode, "decode") == 0) {
        char* output = decode(input, size);
        string filename = argv[2];
        write_file(filename.substr(0, filename.size() - 8).c_str(), output, size);
    }

    free(input);

    return 0;
}
