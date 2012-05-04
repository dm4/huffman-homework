#include <cstdio>
#include <cstdlib>
#include <map>
#include <algorithm>
#include <vector>

#define MAX_SYMBOL_LENGTH   3
#define MAX_CODEWORD_LENGTH 16
#define DEBUG 0

using namespace std;

typedef struct buildCL {
    int symbol;
    int count;
} BuildCL;

typedef struct cLRecordEntry {
    int s1, s2;
    char cl;
} CLRecordEntry;

typedef struct symbol {
    unsigned int codeword;
    char cl;
    char symbol;
} Symbol;

vector<BuildCL *> symbolCount;
vector<CLRecordEntry *> CLRecord;
vector<Symbol *> symbols;
map<char, Symbol *> symbolsMap;

bool sortByCountDescending(const BuildCL *a, BuildCL *b) {
    return a->count > b->count;
}

void assignCLRecursive(int index, char cl) {
    CLRecordEntry *entry = CLRecord[index];
    entry->cl = cl;
    if (entry->s1 < 0)
        assignCLRecursive(~entry->s1, cl + 1);
    if (entry->s2 < 0)
        assignCLRecursive(~entry->s2, cl + 1);
}

void assignCL() {
    assignCLRecursive(CLRecord.size() - 1, 1);
}

void buildSymbols() {
    for (int i = CLRecord.size() - 1; i >= 0; i--) {
        CLRecordEntry *entry = CLRecord[i];
        if (entry->s1 >= 0) {
            Symbol *s = (Symbol *)malloc(sizeof(Symbol));
            s->cl = entry->cl;
            s->symbol = entry->s1;
            symbols.push_back(s);
        }
        if (entry->s2 >= 0) {
            Symbol *s = (Symbol *)malloc(sizeof(Symbol));
            s->cl = entry->cl;
            s->symbol = entry->s2;
            symbols.push_back(s);
        }
    }
}

void buildSymbolsCodeword() {
    unsigned int codeword = 0;
    symbols[0]->codeword = codeword;
    for (int i = 1; i < symbols.size(); i++) {
        int lengthDiff = symbols[i]->cl - symbols[i - 1]->cl;
        codeword += 1;
        codeword <<= lengthDiff;
        symbols[i]->codeword = codeword;
    }
}

bool sortSymbolsByCL(const Symbol *a, const Symbol *b) {
    return a->cl < b->cl;
}

void buildSymbolsMap() {
    for (int i = 0; i < symbols.size(); i++) {
        symbolsMap[symbols[i]->symbol] = symbols[i];
    }
}

void p(unsigned int a, char cl) {
    unsigned int mask = 1 << (cl - 1);
    for (int i = 0; i < cl; i++) {
        printf("%d", (a & mask)?1:0);
        mask >>= 1;
    }
    printf("\n");
}

void appendBuffer(unsigned int *buf, int bufLen, unsigned int codeword, char cl) {
    int shift = 32 - cl - bufLen;
    (*buf) |= (codeword << shift);
}

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "    %s input.txt output.bin\n", argv[0]);
        exit(1);
    }

    // Read input
    printf("Read input file: %s\n", argv[1]);
    map<char, int> count;
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Invalid file: %s\n", argv[1]);
        exit(1);
    }
    while(!feof(fp)) {
        char c;
        int size = fread(&c, 1, 1, fp);
        if (size == 0)
            break;
        count[c]++;
    }
    count['\0']++;

    // sort and convert to symbolCount
    map<char, int>::iterator it;
    for (it = count.begin(); it != count.end(); it++) {
        BuildCL *tmp = (BuildCL *)malloc(sizeof(BuildCL));
        tmp->symbol = it->first;
        tmp->count  = it->second;
        symbolCount.push_back(tmp);
    }
    sort(symbolCount.begin(), symbolCount.end(), sortByCountDescending);

    // print
    if (DEBUG) {
        for (int i = 0; i < symbolCount.size(); i++) {
            printf("%3d => %d\n", symbolCount[i]->symbol, symbolCount[i]->count);
        }
    }

    // merge to build CLRecord
    int CLRIndex = 0;
    while(symbolCount.size() > 1) {
        int size = symbolCount.size();
        BuildCL *s1 = symbolCount[size - 1];
        BuildCL *s2 = symbolCount[size - 2];
        BuildCL *newS = (BuildCL *)malloc(sizeof(BuildCL));
        newS->count = s1->count + s2->count;
        newS->symbol = ~CLRIndex;
        CLRIndex++;
        symbolCount.pop_back();
        symbolCount.pop_back();
        symbolCount.push_back(newS);
        sort(symbolCount.begin(), symbolCount.end(), sortByCountDescending);
        CLRecordEntry *entry = (CLRecordEntry *)malloc(sizeof(CLRecordEntry *));
        entry->s1 = s1->symbol;
        entry->s2 = s2->symbol;
        entry->cl = 0;
        CLRecord.push_back(entry);
        free(s1);
        free(s2);
    }
    free(symbolCount[0]);
    symbolCount.pop_back();

    assignCL();
    buildSymbols();
    sort(symbols.begin(), symbols.end(), sortSymbolsByCL);
    buildSymbolsCodeword();
    buildSymbolsMap();

    // open output file
    printf("Open output file: %s\n", argv[2]);
    FILE *fout = fopen(argv[2], "wb");
    if (fout == NULL) {
        fprintf(stderr, "Couldn't open file: %s\n", argv[2]);
        exit(1);
    }

    // write symbols
    char n = symbols.size();
    fwrite(&n, sizeof(char), 1, fout);
    for (int i = 0; i < symbols.size(); i++) {
        fwrite(symbols[i], sizeof(Symbol), 1, fout);
    }

    // read from start of input file
    fseeko(fp, 0, SEEK_SET);
    unsigned int buf = 0;
    int bufLen = 0;
    while(!feof(fp)) {
        char c;
        int size = fread(&c, 1, 1, fp);
        if (size == 0)
            break;
        Symbol *s = symbolsMap[c];
        appendBuffer(&buf, bufLen, s->codeword, s->cl);
        bufLen += s->cl;
        while (bufLen >= 8) {
            unsigned int remain = buf >> 24;
            char r = remain;
            fwrite(&r, sizeof(char), 1, fout);
            buf <<= 8;
            bufLen -= 8;
        }
    }
    Symbol *s = symbolsMap[0];
    appendBuffer(&buf, bufLen, s->codeword, s->cl);
    bufLen += s->cl;
    while (bufLen > 0) {
        unsigned int remain = buf >> 24;
        char r = remain;
        fwrite(&r, sizeof(char), 1, fout);
        buf <<= 8;
        bufLen -= 8;
    }

    fclose(fp);
    fclose(fout);

    // print
    if (DEBUG) {
        for (int i = 0; i < symbols.size(); i++) {
            printf("symbol %d\tcl %d\t", symbols[i]->symbol, symbols[i]->cl);
            p(symbols[i]->codeword, symbols[i]->cl);
        }
    }

    // free memory
    for (int i = 0; i < CLRecord.size(); i++) {
        free(CLRecord[i]);
    }
    for (int i = 0; i < symbols.size(); i++) {
        free(symbols[i]);
    }

    return 0;
}
