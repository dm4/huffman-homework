#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <map>

#define MAX_SYMBOL_LENGTH   3
#define MAX_CODEWORD_LENGTH 16

using namespace std;

typedef struct symbol {
    char symbol[MAX_SYMBOL_LENGTH + 1];
    char codeword[MAX_CODEWORD_LENGTH + 1];
    int  cl;
} Symbol;

typedef struct lookup {
    int     next[16];
    bool    isSymbol[16];
} Lookup;

vector<Symbol *> symbols;
vector<Lookup *> LUT;

bool cmpSymbol(const Symbol *a, const Symbol *b) {
    if (a->cl != b->cl)
        return a->cl < b->cl;
    return strcmp(a->codeword, b->codeword) == -1;
}

int calcBase(int height, char *codeword) {
    unsigned int mask = 1 << 3;
    int sum = 0;
    for (int i = height - 4; codeword[i]; i++) {
        if (codeword[i] == '1')
            sum += mask;
        mask >>= 1;
    }
    return sum;
}

int bin2dec(int from, int to, char *codeword) {
    unsigned int mask = 1 << (to - from - 1);
    int sum = 0;
    for (int i = from; i < to; i++) {
        if (codeword[i] == '1')
            sum += mask;
        mask >>= 1;
    }
    return sum;
}

void print_codeword(char *codeword) {
    for (int i = 0; codeword[i]; i++) {
        if (i % 4 == 0)
            putchar(' ');
        putchar(codeword[i]);
    }
}

void alignBuf(char buf[5]) {
    int index = 0;
    int num = 0;
    for (int i = 0; i < 4; i++) {
        if (buf[i]) {
            buf[index++] = buf[i];
            num++;
        }
    }
    for (int i = num; i < 4; i++) {
        buf[i] = 0;
    }
}

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "    %s huff_table_name compressed.txt decoded_symbol.txt\n", argv[0]);
        exit(1);
    }

    // read symbol-codeword table to construct symbol vector
    printf("Read symbol-codeword table from file: %s\n", argv[1]);
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Invalid file: %s\n", argv[1]);
        exit(1);
    }
    Symbol tmp;
    while (fscanf(fp, "%s\n%s\n", tmp.symbol, tmp.codeword) != EOF) {
        Symbol *ptr = (Symbol *)malloc(sizeof(Symbol));
        strcpy(ptr->symbol, tmp.symbol);
        strcpy(ptr->codeword, tmp.codeword);
        ptr->cl = strlen(tmp.codeword);
        symbols.push_back(ptr);
    }
    fclose(fp);

    // sort
    printf("Sort by codewords\n");
    sort(symbols.begin(), symbols.end(), cmpSymbol);

    // LUT and ST
    printf("Construct LUT & ST\n");
    map<int, int> buildLUT;
    int height = 0;
    int previousBuildLUT = -1;
    for (int i = 0; i < symbols.size(); i++) {
        Symbol *p = symbols[i];
        if (p->cl > height) {
            height += 4;
        }

        // build new lookup
        int nowBuildLUT = bin2dec(0, height - 4, p->codeword);
        if (nowBuildLUT != previousBuildLUT) {
            Lookup *ptr = (Lookup *)malloc(sizeof(Lookup));
            LUT.push_back(ptr);
            buildLUT[nowBuildLUT] = LUT.size() - 1;
            previousBuildLUT = nowBuildLUT;
//             printf("buildLUT[%d] = %lu\t", nowBuildLUT, LUT.size() - 1);

            // set link
            if (height >= 8) {
                int previousLUT = buildLUT[bin2dec(0, height - 8, p->codeword)];
                Lookup *previous = LUT[previousLUT];
                int index = bin2dec(height - 8, height - 4, p->codeword);
                previous->isSymbol[index] = false;
                previous->next[index] = LUT.size() - 1;
//                 printf("previousLUT %d", previousLUT);
            }
//             printf("\n");
        }

        // set lookup entries
        int depth = height - p->cl;
        int subNum  = 1 << depth;
        // 1111 1111 10, base is 8
        int base = calcBase(height, p->codeword);
        for (int j = 0; j < subNum; j++) {
            LUT[LUT.size() - 1]->isSymbol[base + j] = true;
            LUT[LUT.size() - 1]->next[base + j] = i;
        }
    }

    // print LUT
    if (1) {
        for (int i = 0; i < LUT.size(); i++) {
            printf("LUT[%d]\n", i);
            for (int j = 0; j < 16; j++) {
                Lookup *ptr = LUT[i];
                if (ptr->isSymbol[j]) {
                    printf("[%2d] %s %s\n", j, symbols[ptr->next[j]]->symbol, symbols[ptr->next[j]]->codeword);
                }
                else {
                    printf("[%2d] LUT[%d]\n", j, ptr->next[j]);
                }
            }
        }
    }

    // read from compressed text file
    printf("Read compressed text from file: %s\n", argv[2]);
    FILE *cfp = fopen(argv[2], "r");
    if (cfp == NULL) {
        fprintf(stderr, "Invalid file: %s\n", argv[2]);
        exit(1);
    }
    FILE *fout = fopen(argv[3], "w");
    if (fout == NULL) {
        fprintf(stderr, "Couldn't open file: %s\n", argv[3]);
        exit(1);
    }
    char buf[5] = {0};
    int  nowLUT = 0;
    int  nowCL  = 0;
    while(!feof(cfp)) {
        // align buffer to left
        alignBuf(buf);

        // read from compressed text
        bool isEOF = false;
        for (int i = 0; i < 4; i++) {
            if (buf[i] == 0) {
                int c;
                while (c = fgetc(cfp)) {
                    if (c == '0' || c == '1' || c == EOF)
                        break;
                }
                if (c == EOF) {
                    isEOF = true;
                    break;
                }
                buf[i] = c;
            }
        }
        if (isEOF && strlen(buf) == 0)
            break;

        //
        int index = bin2dec(0, 4, buf);
        printf("Bufffer %s\n", buf);
        Lookup *table = LUT[nowLUT];
        if (table->isSymbol[index]) {
            Symbol *s = symbols[table->next[index]];
            printf("Symbol  %s\n", s->symbol);
            fprintf(fout, "%s\n", s->symbol);
            int shift = s->cl - nowCL;
            for (int i = 0; i < shift; i++) {
                buf[i] = 0;
            }
            nowCL  = 0;
            nowLUT = 0;
        }
        else {
            printf("Jump to LUT[%d]\n", table->next[index]);
            nowLUT = table->next[index];
            nowCL += 4;
            for (int i = 0; i < 4; i++) {
                buf[i] = 0;
            }
        }
    }
    fclose(cfp);
    fclose(fout);

    // deconstruct symbol vector
    for (int i = 0; i < symbols.size(); i++) {
        free(symbols[i]);
    }

    // deconstruct LUT
    for (int i = 0; i < LUT.size(); i++) {
        free(LUT[i]);
    }
    return 0;
}
