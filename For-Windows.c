#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>

int main(int argc, char *argv[]) {

    /* ---------- arguments check ---------- */
    if (argc < 2) {
        fprintf(stderr, "Use: %s <file.bf> [-o output]\n", argv[0]);
        return 1;
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
        "    unsigned char *end = mem + 30000;\n\n"
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
                    fprintf(stderr, "BF Syntax Error : ']' without '['\n");
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
        fprintf(stderr, "BF Syntax Error : '[' without ']'\n");
        fclose(bffile);
        fclose(bffileo);
        return 1;
    }

    /* ---------- End of generated C ---------- */
    fprintf(bffileo, "\n    return 0;\n}\n");

    fclose(bffile);
    fclose(bffileo);

    /* ---------- compilation preparation ---------- */
    char cmd[512];

    if (argc >= 4 && strcmp(argv[2], "-o") == 0) {
        snprintf(cmd, sizeof(cmd),
            "gcc output-c.c -Wall -Wextra -O2 -o \"%s\"",
            argv[3]);
    } else {
        char exe_name[MAX_PATH] = {0};
        const char *filename = strrchr(argv[1], '\\');
        filename = filename ? filename + 1 : argv[1];

        strncpy(exe_name, filename, MAX_PATH - 1);
        char *dot = strrchr(exe_name, '.');
        if (dot) *dot = '\0';

        snprintf(cmd, sizeof(cmd),
            "gcc output-c.c -Wall -Wextra -O2 -o \"%s.exe\"",
            exe_name);
    }

    /* ---------- gcc run ---------- */
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};

    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "CreateProcess failed (%lu)\n", GetLastError());
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exit_code != 0) {
        fprintf(stderr, "gcc has failed (code %lu)\n", exit_code);
        return 1;
    }

    return 0;
}
