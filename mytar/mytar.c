#include <stdio.h>
#include <ctype.h>
#include <string.h>

int isequal(char arg_file_name[], char file_name[]);
int isprefix(char argv[], char name[]);
int issuffix(char argv[], char name[]);

int ascii_to_decimal(char size[], int len);
int roundup_to_multiple(int decimal, int multiple);
int power(int base, int exp);

int iszeroblock(char header[]);

int main(int argc, char *argv[]) {
    if (!(argc >= 2)) {
        fflush(stdout);
        fprintf(stderr, "mytar: need at least one option\n");
        return (2);
    }
    int list_argument;
    int tarfile_argument;
    int f_arg_present = 0;
    for (int i = 1; i < argc; i++) {
        switch (argv[i][0]) {
        case '-':
            switch (argv[i][1]) {
            case 'f':
                if (i == argc - 1) {
                    fflush(stdout);
                    fprintf(stderr, "mytar: option requires an argument -- %s\n", argv[i]);
                    return (64);
                }
                if ((i + 1) < argc && (strcmp(argv[i + 1], "-t") == 0)) {
                    fflush(stdout);
                    fprintf(stderr, "mytar: You must specify one of the options\n");
                    return (2);
                }
                tarfile_argument = i + 1;
                f_arg_present = 1;
                break;
            case 't':
                list_argument = i + 1;
                break;
            default:
                fflush(stdout);
                fprintf(stderr, "mytar: Unknown option: %s\n", argv[i]);
                return (2);
                break;
            }
            break;
        default:
            break;
        }
    }
    if (f_arg_present == 0) {
        fflush(stdout);
        fprintf(stderr, "mytar: Refusing to read archive contents from terminal (missing -f option?)\n");
        fflush(stdout);
        fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
        return (2);
    }
    FILE *file = fopen(argv[tarfile_argument], "r");
    if (file == NULL) {
        fflush(stdout);
        fprintf(stderr, "mytar: %s: Cannot open: No such file or directory\n", argv[tarfile_argument]);
        fflush(stdout);
        fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
        return (2);
    } else {
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
        // int start_zero = 0;
        // int finish_zero = 0;

        int char_count = 0;

        int first_time = 0;

        int d;
        char header[512];
        char name[100];
        char size[12];
        char typeflag;
        int start = 0;
        int block_no = 0;
        // int entered = 0;
        while (1) {
            if (file == NULL) {
                break;
            }
            // entered = 0;
            while (start < 512 && (d = fgetc(file)) != EOF) {
                header[start] = d;
                start += 1;
                char_count += 1;
                // entered = 1;
            }
            block_no += 1;

            // printf("%d\n", start);
            
            if (iszeroblock(header)) {
                
                FILE *p = file;
                fseek(p, 512, SEEK_SET);
                if (!first_time) {
                    first_time = 1;
                    for (int i = 0; i < 512; ++i) {
                        if ((d = fgetc(p)) != '\0' && block_no != 10) {
                            printf("mytar: A lone zero block at 22\n");//, block_no);
                            break;
                            // return (0);
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
                    return (2);
                }
                if ((char_count % 512) != 0) {
                    fflush(stdout);
                    fprintf(stderr, "mytar: Unexpected EOF in archive\n");
                    fflush(stdout);
                    fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
                    return (2);
                }
                // if (start != 0) {
                //     fflush(stdout);
                //     fprintf(stderr, "mytar: Unexpected EOF in archive\n");
                //     fflush(stdout);
                //     fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
                //     return (2);
                // }
                break;
            }

            if (iszeroblock(header)) {
                no_zero += 1;
                // if (no_zero == 1) {
                //     start_zero = offset;
                // }
                // if (no_zero == 2) {
                //     finish_zero = offset;
                // }
            }

            for (int i = 0; i < 100; ++i) {
                name[i] = header[i];
            }
            for (int i = 124; i < 136; ++i) {
                size[i - 124] = header[i];
            }
            typeflag = header[156];


            // Check zero block
            
            if (typeflag != '0' && typeflag != '\0') {
                fflush(stdout);
                fprintf(stderr, "mytar: Unsupported header type: %d\n", typeflag);
                return (2);
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
                    if (isequal(argv[q], name) || isprefix(argv[q], name) || issuffix(argv[q], name)) {
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
            
            // Checking truncated archive
            

            
            int size_len = sizeof(size) / sizeof(size[0]);
            offset += 512;
            offset += roundup_to_multiple(ascii_to_decimal(size, size_len), 512);
            start = 0;
            fseek(file, offset, SEEK_SET);
            // if (ftell(file) == offset) {
            //     fflush(stdout);
            //     fprintf(stderr, "mytar: Unexpected EOF in archive\n");
            //     fflush(stdout);
            //     fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
            //     return (2);
            // }
            // if (file == NULL) {
            //     // break;
            //     fflush(stdout);
            //     fprintf(stderr, "mytar: Unexpected EOF in archive\n");
            //     fflush(stdout);
            //     fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
            //     return (2);
            // }    
        }
        // if (no_zero == 1) {
        //     printf("mytar: A lone zero block at %d\n", no_zero);
        // }
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
                return (2);
            }
        }
    }
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

int isequal(char arg_file_name[], char file_name[]) {
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

int isprefix(char arg_file_name[], char file_name[]) {
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

int issuffix(char arg_file_name[], char file_name[]) {
    // int i = 0;
    // while (arg_file_name[i] != '\0') {
    //     i += 1;
    // }
    if (arg_file_name[0] != '*') {
        return 0;
    }
    return isequal(arg_file_name[1], file_name)
    // int p = 0;
    // while (file_name[p] != '\0') {
    //     p += 1;
    // }
    // p -= 1;
    // for (int j = i - 1; i > 0; i--) {
    //     if (p < 0) {
    //         return 0;
    //     }
    //     if (arg_file_name[j] != file_name[p]) {
    //         return 0;
    //     }
    //     p -= 1;
    // }
    // return 1;
}

int iszeroblock(char header[]) {
    for (int i = 0; i < 512; ++i) {
        if (header[i] != '\0') {
            return 0;
        }
    }
    return 1;
}