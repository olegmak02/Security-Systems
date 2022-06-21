#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <bitset>
#include <vector>

using namespace std;

typedef unsigned long long LL;

LL fastPowMod(LL base, LL p, LL mod) {
    LL ret = 1;
    base %= mod;
    do {
        if (p & 1) ret = ret * base % mod;
        base = base * base % mod;
    } while (p >>= 1);
    return ret;
}

bool MillerRabin(LL n, LL a) {
    if (n == 2 || n == a) return true;
    if (!(n & 1)) return false;
    LL q = n - 1, cnt = 0;
    while (!(q & 1)) q >>= 1, cnt++;
    LL tmp = fastPowMod(a, q, n);
    if (tmp == 1) return true;
    for (LL i = 0; i < cnt; i++, tmp = tmp * tmp % n)
        if (tmp == n - 1) return true;
    return false;
}

bool isPrime(LL n) {
    if (n < 2) return false;
    else if (n == 2) return true;
    LL p[] = { 2, 3, 13, 61, 127, 4013, 9973, 11939, 39119, 193939, 393919, 1001947, 6012491, 20015447, 100016689, 500016311 };
    for (LL i = 0; i < 16; i++)
        if (!MillerRabin(n, p[i])) return false;
    return true;
}

LL ipow(int base, int exp) {
    LL result = 1;
    while (exp)
    {
        result *= (LL)base;
        exp--;
    }
    return result;
}

LL modOfProd(LL first, LL second, LL div) {
    LL min = 0;
    LL max = 0;
    if (first - second < 0) {
        min = first;
        max = second;
    }
    else {
        min = second;
        max = first;
    }
    bitset<64> b(min);
    int e = 64;
    while (b[e - 1] != 1) {
        e--;
    }

    vector<LL> mods;
    mods.push_back(max % div);
    for (int i = 1; i < e; i++) {
        mods.push_back(mods[i - 1] * 2 % div);
    }
    LL res = 0;
    for (LL i = 0; i < mods.size(); i++) {
        res = (mods[i] * b[i] + res) % div;
    }
    return res;
}

LL llrand() {
    LL r = 0;

    for (int i = 0; i < 5; ++i) {
        r = (r << 15) | (rand() & 0x7FFF);
    }
    return r;
}

LL getPrimeNumber(LL lim) {
    LL a = llrand() % lim;
    while (!isPrime(a)) {
        a = llrand() % lim;
    }
    return a;
}

int main() {
    time_t start = time(NULL);
    srand(time(NULL));

    for (int i = 0; i < 100000; i++) {
        LL a = getPrimeNumber(ipow(2, 28));
    }

    cout << (time(NULL) - start) << "sec to generate 100,000 long prime numbers" << endl;
    
    return 0;
 }
