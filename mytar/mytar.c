#include <stdio.h>
#include <string.h>

int isprefix(char *argv, char name[]);
int issuffix(char *argv, char name[]);
int todecimal(char size[]);
int roundup(int decimal);
int power(int base, int exp);

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
        int final_list_argument = list_argument;
        while (final_list_argument + 1 < argc) {
            if (strcmp(argv[final_list_argument + 1], "-f") == 0) {
                break;
            }
            final_list_argument += 1;
        }
        int list_arg_present;
        if (list_argument >= argc) {
            list_arg_present = 0;
        } else if (strcmp(argv[list_argument], "-f") == 0) {
            list_arg_present = 0;
        } else {
            list_arg_present = 1;
        }
        int print_file[final_list_argument - list_argument + 1];

        int offset = 0;

        char d;
        char header[512];
        char name[100];
        char size[12];
        char typeflag;
        FILE *file_pointer = file;
        int start = 0;
        while (1) {
        
            while ((d = fgetc(file)) != EOF && start < 512) {
                header[start] = d;
                start += 1;
            }

            for (int i = 0; i < 100; ++i) {
                name[i] = header[i];
            }
            for (int i = 124; i < 136; ++i) {
                size[i - 124] = header[i];
            }
            typeflag = header[156];

            if (d == EOF) {
                break;
            }

            // Check zero block
            
            if (typeflag != '0' && typeflag != '\0') {
                fprintf(stderr, "mytar: Unsupported header type: %c\n", typeflag);
                return (2);
            }
            if (!list_arg_present) {
                printf("%s\n", name);
            }
            if (list_arg_present) {
                for (int q = list_argument; q <= final_list_argument; q++) {
                    if ((strcmp(argv[q], name) == 0) || isprefix(argv[q], name) || issuffix(argv[q], name)) {
                        print_file[q - list_argument] = 1; 
                    }
                }
            }

            // Checking truncated archive
            

            // Moving pointer

            // printf("%d", todecimal(size));
            // printf("%d", roundup(todecimal(size)));
            fseek(file, roundup(todecimal(size)) + offset, SEEK_SET);
            offset += roundup(todecimal(size));       
        }
        if (list_arg_present) {
            for (int i = list_argument; i <= final_list_argument; i++) {
                if (print_file[i - list_argument]) {
                    printf("%s\n", argv[i]);
                }
            }
            for (int i = list_argument; i < final_list_argument; i++) {
                int fail = 0;
                if (!print_file[i - list_argument]) {
                    fprintf(stderr, "mytar: %s: Not found in archive\n", argv[i]);
                    fail = 1;
                }
                if (fail) {
                    fprintf(stderr, "mytar: Exiting with faliure status due to previous errors\n");
                }
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
    for (int j = i - 1; i > 0; i--) {
        if (argv[j] != name[j]) {
            return 0;
        }
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
    if (decimal <= 512) {
        return 512;
    }
    return 512 + roundup(decimal - 512);
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