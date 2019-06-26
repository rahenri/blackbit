#include <iostream>
#include <stdint.h>

using namespace std;

const unsigned int REP = 4000000000ul;

typedef uint64_t BB;

int main(void) {

    BB a1 = rand() * rand(), a2 = (rand() * rand()) | 1;

    uint32_t b1 = rand(), b2 = rand();

    for(unsigned int i=0;i<REP;++i) {
        a1 = (int(a1 & 0xffffffff) * b1) | ((int(a1 >> 32) * b2) >> 16);
        //a1 = a1 * a2;
    }

    cout << a1 << endl;

    return 0;
}
