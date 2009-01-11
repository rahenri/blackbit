#include <iostream>
#include <stdint.h>
#include <algorithm>

using namespace std;

//typedef uint64_t BB;


#define _min(a, b) ((a)<(b)?(a):(b))

const int TAM = 1<<13;

const int REP = 100000;

struct BB {
    union {
        struct {
            uint32_t a, b;
        };
        uint64_t c;
    };
};

BB vetor[TAM];

uint8_t table[0x10000];


inline int ctz(BB b) {
    /*
    return _min(_min((table[b.a&0xffff]), (table[b.a>>16]+(16))), _min((table[b.b&0xffff]+(32)), (table[b.b>>16]+(48))));
    */
    if(b.a) {
        if(b.a&0xffff)
            return table[b.a&0xffff];
        else
            return table[b.a>>16]+16;
    } else {
        if(b.b&0xffff)
            return table[b.b&0xffff]+32;
        else
            return table[b.b>>16]+48;
    }
}

int main(void) {
    int resp = 0;

    for(int i = 0; i < TAM; ++i) {
        int b = rand() % 64;
        /*
        if(b<32) {
            vetor[i].a = 1 << (b);
            vetor[i].b = 0;
        } else {
            vetor[i].a = 0;
            vetor[i].b = 1 << (b-32);
        }
        */
        vetor[i].c = 1ull << b;
        resp += b;
    }

    for(int i=0;i<0x10000;++i) {
        table[i] = __builtin_ctz(i);
    }

    int t = 0;
    for(int i = 0; i < REP; ++i) {
        for(int j=0;j<TAM;++j) {
            t += __builtin_ctzll(vetor[j].c);
            //t += ctz(vetor[j]);
        }
    }

    cout << t << endl;
    cout << (resp*REP) << endl;

    return 0;
}
