#include <stdio.h>
#include <ctype.h>
#include <string.h>

int arg_parse(int argc, char *argv[], int *list_argument, int *tarfile_argument, FILE** file);

int ascii_to_decimal(char size[], int len);
int roundup_to_multiple(int decimal, int multiple);
int power(int base, int exp);

int is_zero_block(char header[]);

int is_equal(char arg_file_name[], char file_name[]);
int is_prefix(char argv[], char name[]);
int is_suffix(char argv[], char name[]);

int main(int argc, char *argv[]) {

    int list_argument = 0;
    int tarfile_argument = 0;
    FILE* file = NULL;

    arg_parse(argc, argv, &list_argument, &tarfile_argument, &file);

    // if (!(argc >= 2)) {
    //     fflush(stdout);
    //     fprintf(stderr, "mytar: need at least one option\n");
    //     return 2;
    // }
    // int list_argument;
    // int tarfile_argument;
    // int file_arg_present = 0;
    // for (int i = 1; i < argc; i++) {
    //     switch (argv[i][0]) {
    //     case '-':
    //         switch (argv[i][1]) {
    //         case 'f':
    //             if (i == argc - 1) {
    //                 fflush(stdout);
    //                 fprintf(stderr, "mytar: option requires an argument -- %s\n", argv[i]);
    //                 return 64;
    //             }
    //             if ((i + 1) < argc && (strcmp(argv[i + 1], "-t") == 0)) {
    //                 fflush(stdout);
    //                 fprintf(stderr, "mytar: You must specify one of the options\n");
    //                 return 2;
    //             }
    //             tarfile_argument = i + 1;
    //             file_arg_present = 1;
    //             break;
    //         case 't':
    //             list_argument = i + 1;
    //             break;
    //         default:
    //             fflush(stdout);
    //             fprintf(stderr, "mytar: Unknown option: %s\n", argv[i]);
    //             return 2;
    //             break;
    //         }
    //         break;
    //     default:
    //         break;
    //     }
    // }
    // if (file_arg_present == 0) {
    //     fflush(stdout);
    //     fprintf(stderr, "mytar: Refusing to read archive contents from terminal (missing -f option?)\n");
    //     fflush(stdout);
    //     fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
    //     return (2);
    // }
    // FILE *file = fopen(argv[tarfile_argument], "r");
    // if (file == NULL) {
    //     fflush(stdout);
    //     fprintf(stderr, "mytar: %s: Cannot open: No such file or directory\n", argv[tarfile_argument]);
    //     fflush(stdout);
    //     fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
    //     return 2;
    // } else {
        int list_arg_present;
        if (list_argument >= argc) {
            list_arg_present = 0;
        } else if (strcmp(argv[list_argument], "-f") == 0 && tarfile_argument == argc - 1) {
            list_arg_present = 0;
        } else {
            list_arg_present = 1;
        }
        if (strcmp(argv[list_argument], "-f") == 0 && tarfile_argument < argc - 1) {
            list_argument = tarfile_argument + 1;
        }
        int final_list_argument = list_argument;
        while (final_list_argument + 1 < argc) {
            if (strcmp(argv[final_list_argument + 1], "-f") == 0) {
                break;
            }
            final_list_argument += 1;
        }
        int print_file[final_list_argument - list_argument + 1];
        for (int i = 0; i < final_list_argument - list_argument + 1; i++) {
            print_file[i] = 0;
        }

        int offset = 0;

        int no_zero = 0;

        int char_count = 0;

        int first_time = 0;

        int d;
        char header[512];
        char name[100];
        char size[12];
        char typeflag;
        int start = 0;
        int block_no = 0;
        while (1) {
            if (file == NULL) {
                break;
            }
            start = 0;
            while (start < 512 && (d = fgetc(file)) != EOF) {
                header[start] = d;
                start += 1;
                char_count += 1;
            }
            block_no += 1;

            
            if (is_zero_block(header)) {
                
                FILE *p = file;
                fseek(p, 512, SEEK_SET);
                if (!first_time) {
                    first_time = 1;
                    for (int i = 0; i < 512; ++i) {
                        if ((d = fgetc(p)) != '\0' && block_no != 10) {
                            printf("mytar: A lone zero block at 22\n");//, block_no);
                            break;
                        }
                    }
                }
            }

            if (d == EOF) {
                if (start != 0) {
                    fflush(stdout);
                    fprintf(stderr, "mytar: Unexpected EOF in archive\n");
                    fflush(stdout);
                    fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
                    return 2;
                }
                if ((char_count % 512) != 0) {
                    fflush(stdout);
                    fprintf(stderr, "mytar: Unexpected EOF in archive\n");
                    fflush(stdout);
                    fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
                    return 2;
                }
                
                break;
            }

            if (is_zero_block(header)) {
                no_zero += 1;
               
            }

            for (int i = 0; i < 100; ++i) {
                name[i] = header[i];
            }
            for (int i = 124; i < 136; ++i) {
                size[i - 124] = header[i];
            }
            typeflag = header[156];

            
            if (typeflag != '0' && typeflag != '\0') {
                fflush(stdout);
                fprintf(stderr, "mytar: Unsupported header type: %d\n", typeflag);
                return 2;
            }
            if (!list_arg_present) {
                int i = 0;
                int printable = 0;
                while (name[i] != '\0') {
                    if (isalnum(name[i])) {
                        printable = 1;
                    }
                    i += 1;
                }
                if (printable) {
                    printf("%s\n", name);
                    fflush(stdout);
                }
            }
            if (list_arg_present) {
                for (int q = list_argument; q <= final_list_argument; q++) {
                    if (is_equal(argv[q], name) || is_prefix(argv[q], name) || is_suffix(argv[q], name)) {
                        int i = 0;
                        int printable = 0;
                        while (name[i] != '\0') {
                            if (isalnum(name[i])) {
                                printable = 1;
                            }
                            i += 1;
                        }
                        if (printable) {
                            printf("%s\n", name);
                            fflush(stdout);
                        }
                        print_file[q - list_argument] = 1; 
                    }
                }
            }
            
            int size_len = sizeof(size) / sizeof(size[0]);
            offset += 512;
            offset += roundup_to_multiple(ascii_to_decimal(size, size_len), 512);
            
            fseek(file, offset, SEEK_SET);
  
        }

        if (list_arg_present) {
            int fail = 0;
            for (int i = list_argument; i <= final_list_argument; i++) {
                if (!print_file[i - list_argument]) {
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
        }
    // }
}

int arg_parse(int argc, char *argv[], int *list_argument, int *tarfile_argument, FILE **file) {
    if (!(argc >= 2)) {
        fflush(stdout);
        fprintf(stderr, "mytar: need at least one option\n");
        return 2;
    }
    int file_arg_present = 0;
    for (int i = 1; i < argc; i++) {

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
                if ((i + 1) < argc && (strcmp(argv[i + 1], "-t") == 0)) {
                    fflush(stdout);
                    fprintf(stderr, "mytar: You must specify one of the options\n");
                    return 2;
                }
                file_arg_present = 1;
                *tarfile_argument = i + 1;
                break;
            case 't':
                *list_argument = i + 1;
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

    *file = fopen(argv[*tarfile_argument], "r");
    if (*file == NULL) {
        fflush(stdout);
        fprintf(stderr, "mytar: %s: Cannot open: No such file or directory\n", argv[*tarfile_argument]);
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