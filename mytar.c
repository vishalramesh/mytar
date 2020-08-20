#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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

int advance_offset(char size[], int *offset, int *block_no, FILE* file) {
    int size_len = 12;
    int dec = ascii_to_decimal(size, size_len);
    *offset += 512;
    *offset += roundup_to_multiple(dec, 512);
    *block_no += (roundup_to_multiple(dec, 512) / 512);
    FILE *p = file;
    
    for (int i = 0; i < dec; ++i) {
        int d;
        if ((d = fgetc(p)) == EOF) {
            fprintf(stderr, "mytar: Unexpected EOF in archive\n");
            fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
            return 2;
        }
    }
    fseek(file, *offset, SEEK_SET);
    if (file == NULL) {
        return 2;
    }
    return 0;
}

char get_block(char header[], FILE *file, int *pos) {
    int d;
    int start = 0;
    start = fread(header, 512, 1, file);
    // while (start < 512 && (d = fgetc(file)) != EOF) {
    //     header[start] = d;
    //     start += 1;
    // }
    // start = 512;
    d = header[start - 1];
    if (d == EOF) {
        pos = start - 1;
    } else {
        *pos = start;
    }
    return d;
}

int is_condition(char ar[], char file_name[]) {
    return is_equal(ar, file_name) || is_prefix(ar, file_name) ||
        is_suffix(ar, file_name);
}

void print_list_output(char *argv[], int print_file[], char file_name[], int list_start, int list_end) {

    for (int q = list_start; q <= list_end; q++) {
        if (is_condition(argv[q], file_name)) {
            printf("%s\n", file_name);
            fflush(stdout);
            print_file[q - list_start] = 1; 
        }
    }
}

int print_list_error(char *argv[], int print_file[], int list_start, int list_end) {
    int fail = 0;
    for (int i = list_start; i <= list_end; ++i) {
        if (!print_file[i - list_start]) {
            fprintf(stderr, "mytar: %s: Not found in archive\n", argv[i]);
            fail = 1;
        }
    }
    if (!fail) {
        return 0;
    }
    fprintf(stderr, "mytar: Exiting with failure status due to previous errors\n");
    return 2;
}

int la_present(int argc, char *argv[], int list_start, int file_index) {

    if (list_start >= argc) {
        return 0;
    } else if (strcmp(argv[list_start], "-f") == 0 && file_index == argc - 1) {
        return 0;
    }
    return 1;
}

int get_arg_end(int argc, char *argv[], int list_start) {
    int list_end = list_start;
    while (list_end < argc - 1) {
        if (strcmp(argv[list_end + 1], "-f") == 0) {
            break;
        }
        list_end += 1;
    }
    return list_end;
}

int write_to_file(FILE* file, FILE* create, int *offset, int *block_no, char size[]) {
    int size_len = 12;
    FILE *p = file;

    int dec = ascii_to_decimal(size, size_len);

    *offset += 512;
    *offset += roundup_to_multiple(dec, 512);
   
    *block_no += (roundup_to_multiple(dec, 512) / 512);

    for (int i = 0; i < dec; ++i) {
        int d = '\0'; // Arbitrary
        if ((d = fgetc(p)) == EOF) {
            fprintf(stderr, "mytar: Unexpected EOF in archive\n");
            fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
            return 2;
        }
        fputc(d, create);
        if (create == NULL) {
            return 2;
        }
    }
    fseek(file, *offset, SEEK_SET);
    if (file == NULL) {
        return 2;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    int args_present[4] = {0, 0, 0, 0};

    int file_index = 0;
    int list_start = 0;
    int extract_index = 0;

    if (!(argc >= 2)) {
        fprintf(stderr, "mytar: need at least one option\n");
        return 2;
    }

    for (int i = 1; i < argc; ++i) {

        char ch = argv[i][1]; 

        if (argv[i][0] != '-') {
            continue;
        } else if (ch == 'f') {
            if (i >= argc - 1) {
                fprintf(stderr, "mytar: option requires an argument");
                fprintf(stderr, " -- -%c\n", ch);
                return 64;
            }
            if (i < argc - 1 && (strcmp(argv[i + 1], "-t") == 0)) {
                fprintf(stderr, "mytar: Must specify one of the options\n");
                return 2;
            }
            file_index = i + 1;
            args_present[0] = 1;
        } else if (ch == 't') {
            list_start = i + 1;
            args_present[1] = 1;            
        } else if (ch == 'v') {
            args_present[2] = 1;  
        } else if (ch == 'x') {
            extract_index = i + 1;
            args_present[3] = 1;
        } else {
            fprintf(stderr, "mytar: Unknown option: %c\n", ch);
            return 2;
        }
    }

    if (!args_present[0]) {
        fprintf(stderr, "mytar: Refusing to read archive contents");
        fprintf(stderr, " (missing -f option?)\n");
        fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
        return 2;
    }

    if (args_present[1] && args_present[3]) {
        fprintf(stderr, "mytar: You may not specify more than one option\n");
        return 2;
    }

    if (strcmp(argv[list_start], "-v") == 0) {
        list_start += 1;
    }

    if (strcmp(argv[extract_index], "-v") == 0) {
        extract_index += 1;
    }
    
    if (strcmp(argv[list_start], "-f") == 0 && file_index < argc - 1) {
        list_start = file_index + 1;
    }

    if (strcmp(argv[extract_index], "-f") == 0 && file_index < argc - 1) {
        list_start = file_index + 1;
    }

    FILE *file = fopen(argv[file_index], "r");
    if (file == NULL) {
        fprintf(stderr, "mytar: %s: Cannot open", argv[file_index]);
        fprintf(stderr, ": No such file or directory\n");
        fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
        return 2;
    }
    
    int list_arg_present = la_present(argc, argv, list_start, file_index) && args_present[1];
    int extract_arg_present = la_present(argc, argv, extract_index, file_index) && args_present[3];
   
    int list_end = get_arg_end(argc, argv, list_start);
    int end_extract_index = get_arg_end(argc, argv, extract_index);

    int print_file[list_end - list_start + 1];
    for (int i = 0; i < list_end - list_start + 1; ++i) {
        print_file[i] = 0;
    }

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
        fseek(file, offset + 512, SEEK_SET);
        block_no += 1;

        if (is_zero_block(header)) {
            
            FILE *p = file;
            
            for (int i = 0; i < 512; ++i) {
                // Check partial block here?
                int e;
                if ((e = fgetc(p)) != '\0') {
                    
                    int this_ret = 0;
                    if (list_arg_present) {
                        this_ret = print_list_error(argv, print_file, list_start, list_end);
                    }

                    printf("mytar: A lone zero block at %d\n", block_no); // ???
		            fflush(stdout);
                    return this_ret;
                }
            }

            if (list_arg_present) {
                return print_list_error(argv, print_file, list_start, list_end);
            }

            return 0;
        }

        if (d == EOF) {
            if (pos != 0) {
                fprintf(stderr, "mytar: Unexpected EOF in archive\n");
                fprintf(stderr, "mytar: Error is not recoverable:");
                fprintf(stderr, " exiting now\n");
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
        magic[5] = '\0';
        typeflag = header[156];

        if (strcmp(magic, "ustar") != 0) {
            fprintf(stderr, "mytar: This does not look like a tar archive\n");
            fprintf(stderr, "mytar: Exiting with failure status due to previous errors\n");
            return 2;
        }

        if (typeflag != '0' && typeflag != '\0') {
            fprintf(stderr, "mytar: Unsupported header type: %d\n", typeflag);
            return 2;
        }

        if (!list_arg_present && args_present[1]) {
            printf("%s\n", file_name);
            fflush(stdout);
        }

        if (list_arg_present) {
            print_list_output(argv, print_file, file_name, list_start, list_end);
        }

        if (!extract_arg_present && args_present[3]) {
            if (args_present[2]) {
                printf("%s\n", file_name);
	            fflush(stdout);
            }
            FILE* create = fopen(file_name, "w");
            if (create == NULL) {
                fprintf(stderr, "mytar: %s: ", argv[file_index]);
                fprintf(stderr, "Cannot extract: Error creating file\n");
                fprintf(stderr, "mytar: Error is not recoverable");
                fprintf(stderr, ": exiting now\n");
                return 2;
            }
            int write_ret = write_to_file(file, create, &offset, &block_no, size);
            fclose(create);
            if (write_ret != 0) {
                return write_ret;
            }
            continue;
        }

        if (extract_arg_present) {

            int c = 0;
            for (int q = extract_index; q <= end_extract_index; q++) {
                if (is_condition(argv[q], file_name)) {
                    if (args_present[2]) {
                        printf("%s\n", file_name);
                    }
                    FILE* create = fopen(file_name, "w");
                    if (create == NULL) {
                        fprintf(stderr, "mytar: %s: ", argv[file_index]);
                        fprintf(stderr, "Cannot extract: Error creating file\n");
                        fprintf(stderr, "mytar: Error is not recoverable");
                        fprintf(stderr, ": exiting now\n");
                        return 2;
                    }
                    int write_ret = write_to_file(file, create, &offset, &block_no, size);
                    fclose(create);
                    print_file[q - list_start] = 1; 
                    if (write_ret != 0) {
                        return write_ret;
                    }
                    c = 1;
                    break;
                }
            }
            if (c) {
                continue;
            }

        }
        
        int advance_ret = advance_offset(size, &offset, &block_no, file);
        if (advance_ret == 2) {
            return advance_ret;
        }
    }

    if (list_arg_present) {
        return print_list_error(argv, print_file, list_start, list_end);
    }

    if (extract_arg_present) {

        int fail = 0;
        for (int i = extract_index; i <= end_extract_index; ++i) {
            if (!print_file[i - extract_index]) {
                fprintf(stderr, "mytar: %s: Not found in archive\n", argv[i]);
                fail = 1;
            }
        }
        if (!fail) {
            return 0;
        }
        fprintf(stderr, "mytar: ");
        fprintf(stderr, "Exiting with failure status due to previous errors\n");
        return 2;
    }

    fclose(file);
}
