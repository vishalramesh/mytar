#include <stdio.h>
#include <ctype.h>
#include <string.h>

int isprefix(char *argv, char name[]);
int issuffix(char *argv, char name[]);
int todecimal(char size[]);
int roundup(int decimal);
int power(int base, int exp);
int comp(char ar[], char name[]);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "mytar: need at least one option\n");
        return (2);
    }
    char ch;
    int list_argument;
    int tarfile_argument;
    int f_arg_present = 0;
    for (int i = 1; i < argc; i++) {
        switch ((int)argv[i][0]) {
        case '-':
            ch = (int)argv[i][1];
            switch (ch) {
            case 'f':
                if (i == argc - 1) {
                    fprintf(stderr, "mytar: option requires an argument -- %s\n", argv[i]);
                    return (64);
                }
                if (i + 1 < argc && (strcmp(argv[i + 1], "-t") == 0)) {
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
        fprintf(stderr, "mytar: Refusing to read archive contents from terminal (missing -f option?)\n");
        fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
        return (2);
    }
    FILE *file = fopen(argv[tarfile_argument], "r");
    if (file == NULL) {
        fprintf(stderr, "mytar: %s: Cannot open: No such file or directory\n", argv[tarfile_argument]);
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

        // int list_arg_present;
        // if (list_argument >= argc) {
        //     list_arg_present = 0;
        // } else if (strcmp(argv[list_argument], "-f") == 0) {
        //     list_arg_present = 0;
        // } else {
        //     list_arg_present = 1;
        // }

        int offset = 0;

        int d;
        char header[512];
        char name[100];
        char size[12];
        char typeflag;
        int start = 0;
        while (1) {
            if (file == NULL) {
                break;
            }
            while ((d = fgetc(file)) != EOF && start < 512) {
                header[start] = d;
                start += 1;
            }

            if (d == EOF) {
                if (start != 0) {
                    fprintf(stderr, "mytar: Unexpected EOF in archive\n");
                    fprintf(stderr, "mytar: Error is not recoverable: exiting now\n");
                    return (2);
                }
                break;
            }

            for (int i = 0; i < 100; ++i) {
                name[i] = header[i];
            }
            for (int i = 124; i < 136; ++i) {
                size[i - 124] = header[i];
            }
            typeflag = header[156];

            // printf("%d\n", comp(argv[list_argument], name));
            // printf("%s\n", name);

            // Check zero block
            
            if (typeflag != '0' && typeflag != '\0') {
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
                }
            }
            if (list_arg_present) {
                for (int q = list_argument; q <= final_list_argument; q++) {
                    if (comp(argv[q], name) || isprefix(argv[q], name) || issuffix(argv[q], name)) {
                        int i = 0;
                        int printable = 0;
                        while (name[i] != '\0') {
                            if (isalnum(name[i])) {
                                printable = 1;
                            }
                            i += 1;
                        }
                        if (printable) {
                            printf("list argument %d, final arg %d, printable %d\n", list_argument, final_list_argument, printable);
                            printf("%s\n", name);
                        }
                        // fflush(stdout);
                        print_file[q - list_argument] = 1; 
                    }
                }
            }
            
            // Checking truncated archive
            

            // Moving pointer
            
            offset += (512 + roundup(todecimal(size)));
            start = 0;
            fseek(file, offset, SEEK_SET);
            if (file == NULL) {
                break;
            }
            // fseek(file, roundup(todecimal(size)) + offset, SEEK_SET);
            // offset += roundup(todecimal(size));       
        }
        if (list_arg_present) {
            // for (int i = list_argument; i <= final_list_argument; i++) {
            //     if (print_file[i - list_argument]) {
            //         printf("%s\n", argv[i]);
            //     }
            // }
            int fail = 0;
            for (int i = list_argument; i <= final_list_argument; i++) {
                if (!print_file[i - list_argument]) {
                    fprintf(stderr, "list arg %d, final list arg $d\n", list_argument, final_list_argument);
                    fprintf(stderr, "mytar: %s: Not found in archive\n", argv[i]);
                    fail = 1;
                }
            }
            if (fail) {
                fprintf(stderr, "mytar: Exiting with faliure status due to previous errors\n");
                return (2);
            }
        }
    }
}

int isprefix(char *argv, char name[]) {
    int i = 0;
    while (argv[i] != '\0') {
        i += 1;
    }
    if (argv[i - 1] != '*') {
        return 0;
    }
    for (int j = 0; j < i - 1; j++) {
        if (argv[j] != name[j]) {
            return 0;
        }
    }
    return 1;
}

int issuffix(char *argv, char name[]) {
    int i = 0;
    while (argv[i] != '\0') {
        i += 1;
    }
    if (argv[0] != '*') {
        return 0;
    }
    int p = 0;
    while (name[p] != '\0') {
        p += 1;
    }
    p -= 1;
    for (int j = i - 1; i > 0; i--) {
        if (p < 0) {
            return 0;
        }
        if (argv[j] != name[p]) {
            return 0;
        }
        p -= 1;
    }
    return 1;
}

int todecimal(char size[]) {
    int decimal = 0;
    for (int i = 10; i >= 0; i--) {
        decimal += power(8, (12 - i - 2)) * (int)(size[i] - '0');
    }
    return decimal;
}

int roundup(int decimal) {
    // What about 0 size case?
    // if (decimal <= 512) {
    //     return 512;
    // }
    // return 512 + roundup(decimal - 512);
    int roundup = 0;
    while (decimal > 0) {
        roundup += 512;
        decimal -= 512;
    }
    return roundup;
}

int power(int base, int exp) {
    if (exp == 0) {
        return 1;
    }
    int prod = 1;
    for (int i = 1; i <= exp; ++i) {
        prod *= base;
    }
    return prod;
}

int comp(char ar[], char name[]) {
    int i = 0;
    while (ar[i] != '\0') {
        i += 1;
    }
    int j = 0;
    while (name[j] != '\0') {
        j += 1;
    }
    if (i != j) {
        return 0;
    }
    for (int q = 0; q < i; q++) {
        if (ar[q] != name[q]) {
            return 0;
        }
    }
    // while (ar[i] != '\0') {
    //     if (ar[i] != name[i]) {
    //         return 0;
    //     }
    //     i += 1;
    // }
    return 1;
}