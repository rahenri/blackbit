#include <iostream>
#include <stdint.h>
#include <algorithm>

using namespace std;

//typedef uint64_t BB;


#define _min(a, b) ((a)<(b)?(a):(b))

const int TAM = 1<<13;

const int REP = 100000;

struct BB {
    int a, b;
};


BB vetor[TAM];

uint8_t table[0x10000];


int ctz(BB b) {
    return _min(_min((table[b.a&0xffff]), (table[b.a>>16]+(16))), _min((table[b.a&0xffff]+(32)), (table[b.a>>16]+(48))));
}

int main(void) {
    int t = 0;

    for(int i = 0; i < TAM; ++i) {
        int b =rand() % 64;
        vetor[i].a = 1ull << (b);
        vetor[i].a = 1ull << (b-32);
    }

    for(int i=0;i<0x10000;++i) {
        table[i] = __builtin_ctz(i);
    }

    for(int i = 0; i < REP; ++i) {
        for(int j=0;j<TAM;++j) {
            //t += __builtin_ctzll(vetor[j]);
            t += ctz(vetor[j]);
        }
    }

    cout <<t << endl;

}
