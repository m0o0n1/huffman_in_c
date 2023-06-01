#include "error.h"
#include <string.h>
#include <stdlib.h>

typedef struct _MyMap{
	char byte;
	unsigned int count;
} MyMap;


typedef struct _Huffman_Node{
	char *code;
	char byte;
	unsigned count;
	struct _Huffman_Node *zero;
	struct _Huffman_Node *one;
} Huffman_Node;

// TODO: check this
Huffman_Node huffman_pop(Huffman_Node *nodes, unsigned different_bytes){
	Huffman_Node ret;
	ret.byte = nodes[0].byte;
	ret.count = nodes[0].count;
	ret.zero = nodes[0].zero;
	ret.one = nodes[0].one;

	for(int i = 0; i < different_bytes - 1; i++){
		nodes[i].byte = nodes[i+1].byte;
		nodes[i].count = nodes[i+1].count;
		nodes[i].zero = nodes[i+1].zero;
		nodes[i].one = nodes[i+1].one;
	}

	nodes = (Huffman_Node *)realloc(nodes, sizeof(Huffman_Node) * different_bytes - sizeof(Huffman_Node));

	return ret;
}

void huffman_push(Huffman_Node node, Huffman_Node *nodes, unsigned different_bytes){
	nodes = (Huffman_Node *)realloc(nodes, sizeof(Huffman_Node) * different_bytes + sizeof(Huffman_Node));
	nodes[different_bytes - 1].byte = node.byte;
	nodes[different_bytes - 1].count = node.count;
	nodes[different_bytes - 1].zero = node.zero;
	nodes[different_bytes - 1].one = node.one;

	for(int i = 0; i < different_bytes; i++){
		for(int j = i + 1; j < different_bytes; j++){
			if(nodes[j].count < nodes[i].count){
				char byte = nodes[j].byte;
				unsigned count = nodes[j].count;
				Huffman_Node *z = nodes[j].zero;
				Huffman_Node *o = nodes[j].one;

				nodes[j].byte = nodes[i].byte;
				nodes[j].count = nodes[i].count;
				nodes[j].zero = nodes[i].zero;
				nodes[j].one = nodes[i].one;

				nodes[i].byte = byte;
				nodes[i].count = count;
				nodes[i].zero = z;
				nodes[i].one = o;
			}
		}
	}
}

void assign_bytes_codes(Huffman_Node* node, char *code){
	char *temp;
	unsigned len = strlen(code);
	temp = (char *)malloc(sizeof(char) * len + 2 * sizeof(char));
	memmove(temp, code, len + 1);
	temp[len + 1] = '\0';
	if(node->zero != NULL){
		temp[len] = '0';
		assign_bytes_codes(node->zero, temp);
	}
	if(node->one != NULL){
		temp[len] = '1';
		assign_bytes_codes(node->one, temp);
	}
	if(node->one == NULL && node->zero == NULL){
		node->code = (char *)malloc(sizeof(char) * len + 2 * sizeof(char));
		memmove(node->code, temp, len + 1);
	}
	free(temp);
}

void print_nodes(Huffman_Node *nodes){
	if(nodes->zero != NULL)
		print_nodes(nodes->zero);
	if(nodes->one != NULL)
		print_nodes(nodes->one);
	if(nodes->zero == NULL && nodes->one == NULL)
		printf("Byte %X: %s, len: %u\n", nodes->byte, nodes->code, strlen(nodes->code));
}

char* find_code(char byte, Huffman_Node *nodes){
	if (nodes == NULL) {
        	return NULL;
	}
	if (nodes->byte == byte) {
        	size_t len = strlen(nodes->code);
        	char* temp = (char*)malloc((len + 1) * sizeof(char));
        	strcpy(temp, nodes->code);
        	return temp;
    	}

    	char* temp = find_code(byte, nodes->zero);
    	if (temp != NULL) {
        	return temp;
    	}

    	temp = find_code(byte, nodes->one);
    	return temp;

}
void decode_data(FILE *input_file, FILE *output_file, Huffman_Node *nodes) {
    unsigned char byte;
    Huffman_Node *current_node = nodes;

    while (fread(&byte, sizeof(unsigned char), 1, input_file) == 1) {
        for (int bit = 7; bit >= 0; bit--) {
            if (byte & (1 << bit)) {
                current_node = current_node->one;
            } else {
                current_node = current_node->zero;
            }

            if (current_node->zero == NULL && current_node->one == NULL) {
                fwrite(&(current_node->byte), sizeof(char), 1, output_file);
                current_node = nodes;
            }
        }
    }
}

void free_tree(Huffman_Node *nodes){
	if(nodes->zero != NULL){
		free_tree(nodes->zero);
	} 
	if(nodes->one != NULL){
		free_tree(nodes->one);
	}
	if(nodes->one == NULL && nodes->zero == NULL){
		free(nodes->code);
	}
	free(nodes->zero);
	free(nodes->one);
}


char source_file[] = "source.txt";


int main(int argc, char **argv){
	FILE *fs = fopen(source_file, "rb");
	if(fs == NULL){
		print_error("Can't open a source file for encoding!\n");
		return -1;
	}

	// TODO: Let me suppose that the length of a file is greater than zero
	unsigned int different_bytes = 0;
	MyMap *f_table;
	
	char byte;
	while(!feof(fs)){
		fread(&byte, sizeof(char), 1, fs);
		
		char found = 0;	
		for(int i = 0; i < different_bytes; i++){
			if(f_table[i].byte == byte){
				found = 1;
				f_table[i].count += 1;
				break;
			}	
		}
		if(!found){
			f_table = (MyMap *)realloc(f_table, sizeof(MyMap) * different_bytes + sizeof(MyMap));
			f_table[different_bytes].byte = byte;
			f_table[different_bytes].count = 1;
			different_bytes += 1;
		}
	}


	// Let's sort this dictionary
	for(int i = 0; i < different_bytes; i++){
		for(int j = i + 1; j < different_bytes; j++){
			if(f_table[j].count < f_table[i].count){
				char byte = f_table[j].byte;
				unsigned count = f_table[j].count;
				f_table[j].count = f_table[i].count;
				f_table[j].byte = f_table[i].byte;
				f_table[i].count = count;
				f_table[i].byte = byte;
			}
		}
	}

	printf("  %-15s\n", "Frequency Table");
	printf("  %-15s\n", "---------------");
	printf("%-4s\t\t%-5s\n", "Byte", "Count"); 
	for(int i = 0; i < different_bytes; i++){
		printf("0x%-4x\t\t%-5u\n", f_table[i].byte, f_table[i].count);
	}
	printf("\n");

	// Now create a priority queue
	Huffman_Node *nodes = (Huffman_Node *)malloc(sizeof(Huffman_Node) * different_bytes);
	for(int i = 0; i < different_bytes; i++){
		nodes[i].byte = f_table[i].byte;
		nodes[i].count = f_table[i].count;
		nodes[i].zero = NULL;
		nodes[i].one = NULL;
	}
	int max = different_bytes - 1;	

	for(int i = 0; i < max; i++){
		Huffman_Node first = huffman_pop(nodes, different_bytes);
		Huffman_Node second = huffman_pop(nodes, different_bytes - 1);

		Huffman_Node third;
		third.count = first.count + second.count;
		
		third.zero = (Huffman_Node *)malloc(sizeof(Huffman_Node));
		third.zero->byte = first.byte;
		third.zero->count = first.count;
		third.zero->zero = first.zero;
		third.zero->one = first.one;
	
		third.one = (Huffman_Node *)malloc(sizeof(Huffman_Node));
		third.one->byte = second.byte;
		third.one->count = second.count;
		third.one->zero = second.zero;
		third.one->one = second.one;
		
		huffman_push(third, nodes, different_bytes - 1);
	
		different_bytes -= 1;

	}


	print_success("Successfully generated Huffman's Tree!\n");

	// Assign Huffman's codes to bytes
	char *ch = "\0";
	assign_bytes_codes(nodes, ch);	
	print_success("Successfully assigned codes!\n");
	print_nodes(nodes);



	print_success("Writing compressed data\n");
	FILE *fo = fopen("output.dat", "wb");
	rewind(fs);
	
	
	unsigned char offset = 8;
	unsigned char data_to_write = 0;

	while (fread(&byte, sizeof(byte), 1, fs) == 1) {
		char *code = find_code(byte, nodes);
		size_t code_length = strlen(code);
		for (size_t i = 0; i < code_length; i++) {
			if (offset <= 0) {
				fwrite(&data_to_write, sizeof(unsigned char), 1, fo);
				data_to_write = 0;
				offset = 8;
			}

			if (code[i] == '0') {
				data_to_write = data_to_write & ~(1 << (offset - 1));
			} else if (code[i] == '1') {
				data_to_write = data_to_write | (1 << (offset - 1));
			}

			offset -= 1;
		}
		free(code);
	}

	if (offset < 8 && offset >= 0) {
		fwrite(&data_to_write, sizeof(unsigned char), 1, fo);
	}
	fclose(fo);
	fclose(fs);	
	print_success("Compressed file!\n");

	FILE *fd = fopen("output.dat", "rb");
	FILE *fdo = fopen("source_decoded.txt", "wb");
	
	decode_data(fd, fdo, nodes);

	free_tree(nodes);
	fclose(fd);
	fclose(fdo);
	return 0;
}
