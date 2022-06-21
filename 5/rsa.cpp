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
    } else {
        min = second;
        max = first;
    }
    bitset<64> b(min);
    int e = 64;
    while (b[e-1] != 1) {
        e--;
    }

   vector<LL> mods;
    mods.push_back(max % div);
    for (int i = 1; i < e; i++) {
        mods.push_back(mods[i-1] * 2 % div);
    }
    LL res = 0;
    for (LL i = 0; i < mods.size(); i++) {
        res = (mods[i]*b[i] + res) % div;
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

LL getSecretKey(LL fi, LL e) {
    int i = 0;
    LL iter = (LL)(fi / e);
    LL d = 1;
    LL modul = 0;
    while ((modul = modOfProd(d, e, fi)) != 1) {
        d += iter;
        if ((modul = modOfProd(d, e, fi)) > e) {
            d++;
        }
    }
    return d;
}

LL* getEncryptionKeys(LL a, LL b) {
    LL n = a * b;
    LL fi = (a - 1) * (b - 1);
    cout << "fi: " << fi << endl;
    LL e = getPrimeNumber(10000);
    while (e == fi) {
        e = getPrimeNumber(10000);
    }
    LL d = getSecretKey(fi, e);
    return new LL[3]{ e, d, n };
}

LL modExponent(LL cyp, LL d, LL n) {
    bitset<64> b(d);
    int e = 63;
    while (b[e] != 1) {
        e--;
    }

    vector<LL> mods;
    mods.push_back(cyp);
    for (int i = 0; i < e; i++) {
        mods.push_back(modOfProd(mods[i], mods[i], n));
    }

    LL res = 1;
    for (int i = 0; i <= e; i++) {
        if (b[i] == 1)
            res = modOfProd(res, mods[i], n);
    }

    return res;
}

LL encrypt(string block, LL* key) {
    LL e = key[0];
    LL n = key[1];
    LL val = 0;

    for (int i = 0; i < 6; i++) {
        val <<= 8;
        val += block[i];
    }
    return modExponent(val, e, n);
}

string decrypt(LL cyp, LL* key) {
    LL d = key[0];
    LL n = key[1];

    LL res = modExponent(cyp, d, n);
    
    string dec;
    for (int i = 0; i < 6; i++) {
        cout << (char)(res % 256) << endl;
        dec.insert(dec.begin(), (char)(res % 256));
        res >>= 8;
    }
    return dec;
}

int main() {
    srand(time(NULL));
    cout << "start" << endl;
    
    LL* keys = getEncryptionKeys(getPrimeNumber(ipow(2, 28)), getPrimeNumber(ipow(2, 28)));

    LL public_key[2]{ keys[0], keys[2] };
    LL private_key[2]{ keys[1], keys[2] };

    cout << "public: " << public_key[0] << " and " << public_key[1] << endl;
    cout << "private: " << private_key[0] << " and " << private_key[1] << endl;

    string block = "block";

    LL en = encrypt(block, public_key);
    cout << "encrypted: " << en << endl;

    string dec = decrypt(en, private_key);
    cout << "decrypted: " << dec << endl;

    return 0;
}
