#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int arg_parse(int argc, char *argv[],
              int *list_arg_index, int *file_arg_index, int *extract_arg_index, 
              int args_present[], FILE** file);

int ascii_to_decimal(char size[], int len);
int roundup_to_multiple(int decimal, int multiple);
int power(int base, int exp);

int is_zero_block(char header[]);
void initialise_with_zeros(int print_file[], int len);

int is_equal(char arg_file_name[], char file_name[]);
int is_prefix(char arg_file_name[], char file_name[]);
int is_suffix(char arg_file_name[], char file_name[]);

int get_final_arg_index(int argc, char *argv[], int list_arg_index);
int check_list_arg_present(int argc, char *argv[], int list_arg_index, int file_arg_index);

void print_default_list_output(char file_name[]);

int print_list_arg_error(char *argv[], int print_file[], int list_arg_index, int final_list_arg_index);
void print_list_arg_output(char *argv[], int print_file[], char file_name[], int list_arg_index, int final_list_arg_index);

char get_block(char header[], FILE *file, int *pos);
int advance_offset_and_block(char size[], int *offset, int *block_no, FILE* file);



int main(int argc, char *argv[]) {

    int list_arg_index = 0;
    int file_arg_index = 0;
    int extract_arg_index = 0;

    int args_present[4] = {0, 0, 0, 0}; // Alphabetical order f -> t -> v -> x
    
    FILE* file = NULL;

    int arg_parse_ret;
    if ((arg_parse_ret = arg_parse(argc, argv, &list_arg_index, &file_arg_index, &extract_arg_index, args_present, &file)) != 0) {
        return arg_parse_ret;
    }
    
    int list_arg_present = check_list_arg_present(argc, argv, list_arg_index, file_arg_index) && args_present[1];
    int extract_arg_present = check_list_arg_present(argc, argv, extract_arg_index, file_arg_index) && args_present[3];
   
    int final_list_arg_index = get_final_arg_index(argc, argv, list_arg_index);

    int pf_size = final_list_arg_index - list_arg_index + 1;
    int print_file[pf_size];
    initialise_with_zeros(print_file, pf_size);

    int offset = 0;
    int block_no = 0;

    int d;

    char header[512];
    char file_name[100];
    char size[12];
    char magic[6];
    char typeflag;

    while (file != NULL) {

        int pos = 0;
        d = get_block(header, file, &pos);
        block_no += 1;
        
        if (is_zero_block(header)) {
            
            FILE *p = file;
                    
            for (int i = 0; i < 512; ++i) {
                // Check partial block here?
                int e;
                if ((e = fgetc(p)) != '\0') {
                    
                    int this_ret = 0;
                    if (list_arg_present) {
                        this_ret = print_list_arg_error(argv, print_file, list_arg_index, final_list_arg_index);
                    }

                    printf("mytar: A lone zero block at %d\n", block_no);
                    return this_ret;
                }
            }

            if (list_arg_present) {
                return print_list_arg_error(argv, print_file, list_arg_index, final_list_arg_index);
            }

            return 0;
        }

        if (d == EOF) {
            if (pos != 0) {
                fflush(stdout);
                fprintf(stderr, "mytar: Unexpected EOF in archive\n");
                fflush(stdout);
                fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
                return 2;
            }
            if (pos == 0) {
                return 0;
            }
        }
        
        for (int i = 0; i < 100; ++i) {
            file_name[i] = header[i];
        }
        for (int i = 124; i < 136; ++i) {
            size[i - 124] = header[i];
        }
        for (int i = 257; i < 263; ++i) {
            magic[i - 257] = header[i];
        }
        typeflag = header[156];

        if (magic[0] != 'u' || magic[1] != 's' || magic[2] != 't' || magic[3] != 'a' || magic[4] != 'r') {
            fflush(stdout);
            fprintf(stderr, "mytar: This does not look like a tar archive\n");
            fflush(stdout);
            fprintf(stderr, "mytar: Exiting with failure status due to previous errors\n");
            return 2;
        }

        if (typeflag != '0' && typeflag != '\0') {
            fflush(stdout);
            fprintf(stderr, "mytar: Unsupported header type: %d\n", typeflag);
            return 2;
        }

        if (!list_arg_present && args_present[1]) {
            print_default_list_output(file_name);
        }

        if (list_arg_present) {
            print_list_arg_output(argv, print_file, file_name, list_arg_index, final_list_arg_index);
        }

        if (!extract_arg_present && args_present[3] && !args_present[2]) { // Without -v
            FILE* create_file = fopen(file_name, "w");
            
            fclose(create_file);
        }

        if (!extract_arg_present && args_present[3] && args_present[2]) {
            
        }

        if (extract_arg_present && args_present[3] && !args_present[2]) {
            
        }

        if (extract_arg_present && args_present[3] && args_present[2]) {
            
        }

        int advance_ret = advance_offset_and_block(size, &offset, &block_no, file);
        if (advance_ret == 2) {
            return advance_ret;
        }
    }

    if (list_arg_present) {
        return print_list_arg_error(argv, print_file, list_arg_index, final_list_arg_index);
    }
}

int advance_offset_and_block(char size[], int *offset, int *block_no, FILE* file) {
    int size_len = 12;
    *offset += 512;
    *offset += roundup_to_multiple(ascii_to_decimal(size, size_len), 512);
    *block_no += (roundup_to_multiple(ascii_to_decimal(size, size_len), 512) / 512);
    FILE *p = file;
    
    for (int i = 0; i < ascii_to_decimal(size, size_len); ++i) {
        int d;
        if ((d = fgetc(p)) == EOF) {
            fflush(stdout);
            fprintf(stderr, "mytar: Unexpected EOF in archive\n");
            fflush(stdout);
            fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
            return 2;
        }
    }
    fseek(file, *offset, SEEK_SET);
    return 0;
}

char get_block(char header[], FILE *file, int *pos) {
    int d;
    int start = 0;
    while (start < 512 && (d = fgetc(file)) != EOF) {
        header[start] = d;
        start += 1;
    }
    *pos = start;
    return d;
}

void print_list_arg_output(char *argv[], int print_file[], char file_name[], int list_arg_index, int final_list_arg_index) {

    for (int q = list_arg_index; q <= final_list_arg_index; q++) {
        if (is_equal(argv[q], file_name) || is_prefix(argv[q], file_name) || is_suffix(argv[q], file_name)) {
            int i = 0;
            int printable = 0;
            while (file_name[i] != '\0') {
                if (isalnum(file_name[i])) {
                    printable = 1;
                }
                i += 1;
            }
            if (printable) {
                printf("%s\n", file_name);
                fflush(stdout);
            }
            print_file[q - list_arg_index] = 1; 
        }
    }
}

int print_list_arg_error(char *argv[], int print_file[], int list_arg_index, int final_list_arg_index) {
    int fail = 0;
    for (int i = list_arg_index; i <= final_list_arg_index; ++i) {
        if (!print_file[i - list_arg_index]) {
            fflush(stdout);
            fprintf(stderr, "mytar: %s: Not found in archive\n", argv[i]);
            fail = 1;
        }
    }
    if (fail) {
        fflush(stdout);
        fprintf(stderr, "mytar: Exiting with failure status due to previous errors\n");
        return 2;
    }
    return 0;
}

int arg_parse(int argc, char *argv[],
              int *list_arg_index, int *file_arg_index, int *extract_arg_index, 
              int args_present[], FILE **file) {

    if (!(argc >= 2)) {
        fflush(stdout);
        fprintf(stderr, "mytar: need at least one option\n");
        return 2;
    }
    int file_arg_present = 0;
    for (int i = 1; i < argc; ++i) {

        int char_0 = argv[i][0];
        int char_1 = argv[i][1];

        if (char_0 == '-') {
            switch (char_1) {
            case 'f':
                if (i == argc - 1) {
                    fflush(stdout);
                    fprintf(stderr, "mytar: option requires an argument -- -%c\n", char_1);
                    return 64;
                }
                if (i < argc - 1 && (strcmp(argv[i + 1], "-t") == 0)) {
                    fflush(stdout);
                    fprintf(stderr, "mytar: You must specify one of the options\n");
                    return 2;
                }
                file_arg_present = 1;
                *file_arg_index = i + 1;
                args_present[0] += 1;
                break;
            case 't':
                *list_arg_index = i + 1;
                args_present[1] += 1;
                break;
            case 'v':
                args_present[2] += 1;
                break;
            case 'x':
                *extract_arg_index = i + 1;
                args_present[3] += 1;
                break;
            default:
                fflush(stdout);
                fprintf(stderr, "mytar: Unknown option: %c\n", char_1);
                return 2;
            }
        }   
    }

    if (!file_arg_present) {
        fflush(stdout);
        fprintf(stderr, "mytar: Refusing to read archive contents from terminal (missing -f option?)\n");
        fflush(stdout);
        fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
        return 2;
    }

    // t and x
    if (args_present[1] && args_present[3]) {
        fflush(stdout);
        fprintf(stderr, "mytar: You may not specify more than one option\n");
        return 2;
    }

    if (strcmp(argv[*list_arg_index], "-v") == 0) {
        *list_arg_index += 1;
    }

    if (strcmp(argv[*extract_arg_index], "-v") == 0) {
        *extract_arg_index += 1;
    }
    
    if (strcmp(argv[*list_arg_index], "-f") == 0 && *file_arg_index < argc - 1) {
        *list_arg_index = *file_arg_index + 1;
    }

    if (strcmp(argv[*extract_arg_index], "-f") == 0 && *file_arg_index < argc - 1) {
        *list_arg_index = *file_arg_index + 1;
    }

    *file = fopen(argv[*file_arg_index], "r");
    if (*file == NULL) {
        fflush(stdout);
        fprintf(stderr, "mytar: %s: Cannot open: No such file or directory\n", argv[*file_arg_index]);
        fflush(stdout);
        fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
        return 2;
    }

    return 0;

}

int power(int base, int exp) {
    int prod = 1;
    for (int i = 1; i <= exp; ++i) {
        prod = prod * base;
    }
    return prod;
}

int ascii_to_decimal(char size[], int len) {
    int decimal = 0;
    int digit_count = len - 1; // Not counting terminating '\0' 

    for (int i = 0; i <= digit_count - 1; ++i) {
        int digit = size[i] - '0';
        decimal += digit * power(8, digit_count - 1 - i);
    }

    return decimal;
}

int roundup_to_multiple(int decimal, int multiple) {
    int roundup = 0;
    while (decimal > 0) {
        roundup += multiple;
        decimal -= multiple;
    }
    return roundup;
}

int is_equal(char arg_file_name[], char file_name[]) {
    int i = 0;
    while (arg_file_name[i] != '\0') {
        i += 1;
    }
    int j = 0;
    while (file_name[j] != '\0') {
        j += 1;
    }
    if (i != j) {
        return 0;
    }
    for (int k = 0; k < i; ++k) {
        if (arg_file_name[k] != file_name[k]) {
            return 0;
        }
    }
    return 1;
}

int is_prefix(char arg_file_name[], char file_name[]) {
    int i = 0;
    while (arg_file_name[i] != '\0') {
        i += 1;
    }
    if (arg_file_name[i - 1] != '*') {
        return 0;
    }
    for (int j = 0; j < i - 1; j++) {
        if (arg_file_name[j] != file_name[j]) {
            return 0;
        }
    }
    return 1;
}

int is_suffix(char arg_file_name[], char file_name[]) {
    if (arg_file_name[0] != '*') {
        return 0;
    }
    return is_equal(++arg_file_name, file_name);
}

int is_zero_block(char header[]) {
    for (int i = 0; i < 512; ++i) {
        if (header[i] != '\0') {
            return 0;
        }
    }
    return 1;
}

void initialise_with_zeros(int print_file[], int len) {
    for (int i = 0; i < len; ++i) {
        print_file[i] = 0;
    }
}

void print_default_list_output(char file_name[]) {
    int i = 0;
    int printable = 0;
    while (file_name[i] != '\0') {
        if (isalnum(file_name[i])) {
            printable = 1;
        }
        i += 1;
    }
    if (printable) {
        printf("%s\n", file_name);
        fflush(stdout);
    }
}

int check_list_arg_present(int argc, char *argv[], int list_arg_index, int file_arg_index) {

    if (list_arg_index >= argc) {
        return 0;
    } else if (strcmp(argv[list_arg_index], "-f") == 0 && file_arg_index == argc - 1) {
        return 0;
    }
    return 1;
}

int get_final_arg_index(int argc, char *argv[], int list_arg_index) {
    int final_list_arg_index = list_arg_index;
    while (final_list_arg_index < argc - 1) {
        if (strcmp(argv[final_list_arg_index + 1], "-f") == 0) {
            break;
        }
        final_list_arg_index += 1;
    }
    return final_list_arg_index;
}