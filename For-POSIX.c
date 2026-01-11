#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // fork, execvp
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Use: %s <file.bf> [-o output]\n", argv[0]);
        return 1;
    }

    /* ---------- -o management ---------- */
    const char *output_name = "output-bf";
    if (argc >= 4 && strcmp(argv[2], "-o") == 0) {
        output_name = argv[3];
    }

    FILE *bffile = fopen(argv[1], "r");
    if (!bffile) {
        perror("Fatal Error: Unable to open the BF file");
        return 1;
    }

    FILE *bffileo = fopen("output-c.c", "w");
    if (!bffileo) {
        perror("Compilation Error: Unable to create the C file");
        fclose(bffile);
        return 1;
    }

    /* ---------- C header generation ---------- */
    fprintf(bffileo,
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n\n"
        "int main(void) {\n"
        "    unsigned char mem[30000] = {0};\n"
        "    unsigned char *ptr = mem;\n"
        "    unsigned char *end = mem + 30000;\n\n",
    );

    /* ---------- verifications ---------- */
    int stack = 0;
    int c;

    while ((c = fgetc(bffile)) != EOF) {
        switch (c) {

            case '>':
                fprintf(bffileo,
                    "    if (ptr < end - 1) ptr++;\n");
                break;

            case '<':
                fprintf(bffileo,
                    "    if (ptr > mem) ptr--;\n");
                break;

            case '+':
                fprintf(bffileo, "    ++(*ptr);\n");
                break;

            case '-':
                fprintf(bffileo, "    --(*ptr);\n");
                break;

            case '.':
                fprintf(bffileo, "    putchar(*ptr);\n");
                break;

            case ',':
                fprintf(bffileo,
                    "    int ch = getchar();\n"
                    "    if (ch != EOF) *ptr = (unsigned char)ch;\n");
                break;

            case '[':
                if (++stack > 65535) {
                    fprintf(stderr, "Security Warning: loop nesting too deep, stack overflow risk, compilation stopped\n");
                    fclose(bffile);
                    fclose(bffileo);
                    return 1;
                }
                fprintf(bffileo, "    while (*ptr) {\n");
                break;

            case ']':
                if (stack == 0) {
                    fprintf(stderr, "BF Syntax Error: ']' without '['\n");
                    fclose(bffile);
                    fclose(bffileo);
                    return 1;
                }
                stack--;
                fprintf(bffileo, "    }\n");
                break;
        }
    }

    if (stack != 0) {
        fprintf(stderr, "BF Syntax Error: '[' without ']'\n");
        fclose(bffile);
        fclose(bffileo);
        return 1;
    }

    /* ---------- End of generated C ---------- */
    fprintf(bffileo, "\n    return 0;\n}\n");

    fclose(bffile);
    fclose(bffileo);

    /* ---------- Compilation ---------- */
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // ===== child process =====
        char *gcc_args[] = {
            "gcc",
            "output-c.c",
            "-Wall",
            "-Wextra",
            "-O2",
            "-o",
            (char *)output_name,
            NULL
        };

        execvp("gcc", gcc_args);

        perror("execvp");
        exit(1);
    } else {
        // ===== root prprocess =====
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0) {
                fprintf(stderr, "gcc a échoué (code %d)\n", code);
                exit(1);
            }
        } else {
            fprintf(stderr, "gcc anomrmaly ended\n");
            exit(1);
        }
    }

    return 0;
}
