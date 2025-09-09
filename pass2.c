#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char opcode[20], operand[20], symbol[20], label[20], code[20], mnemonic[25];
    char objectcode[20], progname[20];
    char character;
    int flag, flag1, locctr, loc, startAddr, progLen;

    FILE *fp1, *fp2, *fp3, *fp4, *fp5, *flen;

    // Open input/output files
    fp1 = fopen("intermediate.txt", "r");  // Pass 1 intermediate file
    fp2 = fopen("twoout.txt", "w");        // Pass 2 listing file
    fp3 = fopen("optab.txt", "r");         // Opcode table
    fp4 = fopen("symtab.txt", "r");        // Symbol table
    fp5 = fopen("objprog.txt", "w");       // Object program
    flen = fopen("length.txt", "r");       // Program length file

    if (!fp1 || !fp2 || !fp3 || !fp4 || !fp5 || !flen) {
        printf("Error opening files!\n");
        exit(1);
    }

    // Read program length
    fscanf(flen, "%x", &progLen);
    fclose(flen);

    // Read first line (label, opcode, operand)
    fscanf(fp1, "%s%s%s", label, opcode, operand);

    // Handle START directive
    if (strcmp(opcode, "START") == 0) {
        strcpy(progname, label);
        startAddr = (int)strtol(operand, NULL, 16);
        fprintf(fp2, "%s\t%s\t%s\n", label, opcode, operand);

        // Print header record
        fprintf(fp5, "H^%-6s^%06X^%06X\n", progname, startAddr, progLen);

        // Read next line
        fscanf(fp1, "%x%s%s%s", &locctr, label, opcode, operand);
    }

    // Prepare for text record
    fprintf(fp5, "T^%06X^", locctr);
    long trecordPos = ftell(fp5);   // Save position for length
    fprintf(fp5, "00");             // Placeholder for length
    int tlen = 0;                   // Current text record length

    // Process lines until END
    while (strcmp(opcode, "END") != 0) {
        flag = 0;
        rewind(fp3);

        // Search in opcode table
        while (fscanf(fp3, "%s%s", code, mnemonic) == 2) {
            if ((strcmp(opcode, code) == 0) && (strcmp(mnemonic, "*") != 0)) {
                flag = 1;
                break;
            }
        }

        strcpy(objectcode, "");

        if (flag == 1) {
            flag1 = 0;
            rewind(fp4);

            // Search in symbol table
            while (fscanf(fp4, "%x%s", &loc, symbol) == 2) {
                if (strcmp(symbol, operand) == 0) {
                    flag1 = 1;
                    break;
                }
            }

            if (flag1 == 1) {
                sprintf(objectcode, "%s%04X", mnemonic, loc);
            } else {
                strcpy(objectcode, mnemonic);
            }
        }
        else if (strcmp(opcode, "BYTE") == 0) {
            if (operand[0] == 'C') {
                character = operand[2];
                sprintf(objectcode, "%02X", (int)character);
            }
            else if (operand[0] == 'X') {
                strncpy(objectcode, &operand[2], strlen(operand) - 3);
                objectcode[strlen(operand) - 3] = '\0';
            }
            else {
                sprintf(objectcode, "%02X", atoi(operand));
            }
        }
        else if (strcmp(opcode, "WORD") == 0) {
            sprintf(objectcode, "%06X", atoi(operand));
        }
        else {
            strcpy(objectcode, "");  // RESW, RESB, etc.
        }

        // Write listing file
        fprintf(fp2, "%s\t%s\t%s\t%04X\t%s\n", label, opcode, operand, locctr, objectcode);

        // Handle text record writing
        if (strlen(objectcode) > 0) {
            if (tlen + (strlen(objectcode) / 2) > 30) {
                // Close current text record
                long curr = ftell(fp5);
                fseek(fp5, trecordPos, SEEK_SET);
                fprintf(fp5, "%02X", tlen);
                fseek(fp5, curr, SEEK_SET);

                fprintf(fp5, "\nT^%06X^", locctr);
                trecordPos = ftell(fp5);
                fprintf(fp5, "00");
                tlen = 0;
            }
            fprintf(fp5, "^%s", objectcode);
            tlen += strlen(objectcode) / 2;
        } else {
            if (tlen > 0) {
                // Finish current text record
                long curr = ftell(fp5);
                fseek(fp5, trecordPos, SEEK_SET);
                fprintf(fp5, "%02X", tlen);
                fseek(fp5, curr, SEEK_SET);
                fprintf(fp5, "\n");
                tlen = 0;
            }
        }

        // Read next line
        if (fscanf(fp1, "%x%s%s%s", &locctr, label, opcode, operand) != 4) {
            break;
        }
    }

    // Finalize last text record
    if (tlen > 0) {
        long curr = ftell(fp5);
        fseek(fp5, trecordPos, SEEK_SET);
        fprintf(fp5, "%02X", tlen);
        fseek(fp5, curr, SEEK_SET);
    }

    // End record
    fprintf(fp5, "\nE^%06X\n", startAddr);

    // Close all
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
    fclose(fp5);

    printf("Pass 2 completed successfully!\n");
    printf("Listing file: twoout.txt\n");
    printf("Object program: objprog.txt\n");
    return 0;
}
