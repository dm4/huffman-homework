#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>

#define DEBUG 0

using namespace std;

typedef struct symbol {
    unsigned int codeword;
    char cl;
    char symbol;
} Symbol;

typedef struct lookup {
    int     next[16];
    bool    isSymbol[16];
} Lookup;

vector<Symbol *> symbols;
vector<Lookup *> LUT;

void p(unsigned int a, char cl) {
    unsigned int mask = 1 << (cl - 1);
    for (int i = 0; i < cl; i++) {
        printf("%d", (a & mask)?1:0);
        mask >>= 1;
    }
    printf("\n");
}

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "    %s compressed.bin output.txt\n", argv[0]);
        exit(1);
    }

    // Read input
    printf("Open input file: %s\n", argv[1]);
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Invalid file: %s\n", argv[1]);
        exit(1);
    }

    // Read symbols
    printf("Read symbol-codeword table\n");
    char n;
    fread(&n, sizeof(char), 1, fp);
    for (int i = 0; i < n; i++) {
        Symbol *s = (Symbol *)malloc(sizeof(Symbol));
        fread(s, sizeof(Symbol), 1, fp);
        symbols.push_back(s);
    }

    // Builde LUT
    printf("Construct LUT\n");
    map<int, int> buildLUT;
    int height = 0;
    int previousBuildLUT = -1;
    for (int i = 0; i < symbols.size(); i++) {
        Symbol *p = symbols[i];
        if (p->cl > height) {
            height += 4;
        }

        // build new lookup
        int nowBuildLUT = p->codeword >> (p->cl + 4 - height);
        if (nowBuildLUT != previousBuildLUT) {
            Lookup *ptr = (Lookup *)malloc(sizeof(Lookup));
            LUT.push_back(ptr);
            buildLUT[nowBuildLUT] = LUT.size() - 1;
            previousBuildLUT = nowBuildLUT;

            // set link
            if (height >= 8) {
                int previousLUT = buildLUT[p->codeword >> (p->cl + 8 - height)];
                Lookup *previous = LUT[previousLUT];
                unsigned int index = p->codeword >> (4 + p->cl - height);
                index &= 15;
                previous->isSymbol[index] = false;
                previous->next[index] = LUT.size() - 1;
            }
        }

        // set lookup entries
        int depth = height - p->cl;
        int subNum  = 1 << depth;
        // 1111 1111 10, base is 8
        unsigned int base = p->codeword << (height - p->cl);
        base &= 15;
        for (int j = 0; j < subNum; j++) {
            LUT[LUT.size() - 1]->isSymbol[base + j] = true;
            LUT[LUT.size() - 1]->next[base + j] = i;
        }
    }

    // print
    if (DEBUG) {
        for (int i = 0; i < symbols.size(); i++) {
            printf("symbol %d\tcl %d\t", symbols[i]->symbol, symbols[i]->cl);
            p(symbols[i]->codeword, symbols[i]->cl);
        }
    }

    // print LUT
    if (DEBUG) {
        for (int i = 0; i < LUT.size(); i++) {
            fprintf(stderr, "LUT[%d]\n", i);
            for (int j = 0; j < 16; j++) {
                Lookup *ptr = LUT[i];
                if (ptr->isSymbol[j]) {
                    fprintf(stderr, "[%2d] %d ", j, symbols[ptr->next[j]]->symbol);
                    p(symbols[ptr->next[j]]->codeword, symbols[ptr->next[j]]->cl);
                }
                else {
                    fprintf(stderr, "[%2d] LUT[%d]\n", j, ptr->next[j]);
                }
            }
        }
    }

    // open output file
    printf("Open output file: %s\n", argv[2]);
    FILE *fout = fopen(argv[2], "w");
    if (fout == NULL) {
        fprintf(stderr, "Couldn't open file: %s\n", argv[2]);
        exit(1);
    }

    unsigned char buf, buf2;
    int bufLen, buf2Len;
    int nowLUT = 0;
    int nowCL  = 0;
    fread(&buf, sizeof(char), 1, fp);
    buf2    = 0;
    bufLen  = 8;
    buf2Len = 0;
    while(!feof(fp)) {
        // read to buffer
        while (bufLen < 4) {
            if (bufLen + buf2Len <= 8) {
                buf |= (buf2 >> bufLen);
                bufLen  = bufLen + buf2Len;
                fread(&buf2, sizeof(char), 1, fp);
                buf2Len = 8;
            }
            else {
                buf |= (buf2 >> bufLen);
                buf2 <<= (8 - bufLen);
                buf2Len = bufLen + buf2Len - 8;
                bufLen  = 8;
            }
        }

        //
        char index = (buf >> 4) & 15;
        Lookup *table = LUT[nowLUT];
        if (table->isSymbol[index]) {
            Symbol *s = symbols[table->next[index]];
            if (s->symbol == 0) {
                printf("Done!\n");
                break;
            }
            fprintf(fout, "%c", s->symbol);
            int shift = s->cl - nowCL;
            buf <<= shift;
            bufLen -= shift;
            nowCL  = 0;
            nowLUT = 0;
        }
        else {
            nowLUT = table->next[index];
            nowCL += 4;
            buf <<= 4;
            bufLen -= 4;
        }
    }

    // close file
    fclose(fp);
    fclose(fout);

    // free memory
    for (int i = 0; i < symbols.size(); i++) {
        free(symbols[i]);
    }
    for (int i = 0; i < LUT.size(); i++) {
        free(LUT[i]);
    }

    return 0;
}
