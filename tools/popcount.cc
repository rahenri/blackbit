#include <iostream>
#include <stdint.h>

using namespace std;

//typedef uint64_t BB;
struct BB {
    uint32_t a, b;
};

const int TAM = 1<<13;

BB vetor[TAM];


uint8_t table[0x10000];

const int REP = 60000;

int popcount(BB b) {
    /*int r = 0;
    for(int i=0;i<4;++i) {
        r += table[b&0xffff];
        b >>= 16;
    }
    return r;*/
    return table[b.a&0xffff] +
           table[b.a>>16] +
           table[b.b&0xffff] +
           table[b.b>>16];
}

int main(void) {
    int t = 0;

    for(int i = 0; i < TAM; ++i) {
        //vetor[i] = BB(rand()) * BB(rand());
        vetor[i].a = rand();
        vetor[i].b = rand();
    }

    table[0] = 0;
    for(int i=1;i<0x10000;++i) {
        table[i] = table[i &(i-1)] + 1;
        //table[i] = simple_popcount(i);
    }

    for(int i = 0; i < REP; ++i) {
        for(int j=0;j<TAM;++j) {
            //t += __builtin_popcount(vetor[j].a);
            //t += __builtin_popcount(vetor[j].b);
            t += popcount(vetor[j]);
            //t += simple_popcount(vetor[j]);
        }
    }

    cout << t << endl;

    return 0;
}
