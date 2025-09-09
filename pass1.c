#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    FILE *f1, *f2, *f3, *f4 ,*f5;
    int lc, sa, op1, o, len;
    char m1[20], la[20], op[20], otp[20];

    f1 = fopen("input.txt", "r");
    f3 = fopen("intermediate.txt", "w");
    f4 = fopen("symtab.txt", "w");
    f5=fopen("length.txt","w");
    if (f1 == NULL || f3 == NULL || f4 == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // Read first line
    fscanf(f1, "%s %s %X", la, m1, &op1);

    if (strcmp(m1, "START") == 0) {
        sa = op1;
        lc = sa;
        printf("\t%s\t%s\t%X\n", la, m1, op1);
        fprintf(f3, "\t%s\t%s\t%X\n", la, m1, op1);
    } else {
        lc = 0;
    }

    // Read next line (label + mnemonic)
    fscanf(f1, "%s %s", la, m1);

    while (!feof(f1)) {
        fscanf(f1, "%s", op);

        // Print to console + intermediate file
        printf("\n%X\t%s\t%s\t%s\n", lc, la, m1, op);
        fprintf(f3, "%X\t%s\t%s\t%s\n", lc, la, m1, op);

        // If label exists, write to SYMTAB
        if (strcmp(la, "-") != 0) {
            fprintf(f4, "%X\t%s\n", lc, la);
        }

        // Open OPTAB
        f2 = fopen("optab.txt", "r");
        fscanf(f2, "%s %d", otp, &o);

        while (!feof(f2)) {
            if (strcmp(m1, otp) == 0) {
                lc = lc + 3; // all SIC instructions are 3 bytes
                break;
            }
            fscanf(f2, "%s %d", otp, &o);
        }
        fclose(f2);

        // Handle directives
        if (strcmp(m1, "WORD") == 0) {
            lc = lc + 3;
        } else if (strcmp(m1, "RESW") == 0) {
            op1 = atoi(op);
            lc = lc + (3 * op1);
        } else if (strcmp(m1, "BYTE") == 0) {
            if (op[0] == 'X')
                lc = lc + 1;
            else {
                len = strlen(op) - 3; // remove C'..'
                lc = lc + len;
            }
        } else if (strcmp(m1, "RESB") == 0) {
            op1 = atoi(op);
            lc = lc + op1;
        }

        // Read next line
        fscanf(f1, "%s %s", la, m1);
    }

    // Handle END
    if (strcmp(m1, "END") == 0) {
        printf("\nProgram Length = %X\n", lc - sa);
        fprintf(f5, "%X\n", lc - sa);
    }

    fclose(f1);
    fclose(f3);
    fclose(f4);

    return 0;
}
