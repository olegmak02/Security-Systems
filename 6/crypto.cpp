#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <cmath>
#include <unistd.h>
#include <bitset>
#include <vector>
#include <dirent.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#define TIME_INTERVAL 691200

using namespace std;

typedef unsigned long long LL;

char em_path[200];

char* concat(char* s1, const char* s2) {
	char* res = new char[500];
	strcpy(res, s1);
	strcat(res, s2);
	return res;
}

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

string operator ^(string a, string b) {
    string res;
    for (int i = 0; i < a.size(); i++) {
        res += a[i] ^ b[i];
    }
    return res;
}

LL* getEncryptionKeys(LL a, LL b) {
    LL n = a * b;
    LL fi = (a - 1) * (b - 1);
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

LL stringToLL(string s) {
    LL val = 0;
    for (int i = 0; i < 6; i++) {
        val <<= 8;
        val += s[i];
    }
    return val;
}

string LLToString(LL val) {
    string dec;
    for (int i = 0; i < 6; i++) {
        dec.insert(dec.begin(), (char)(val % 256));
        val >>= 8;
    }
    return dec;
}

LL encryptDecrypt(LL cyp, LL* key) {
    LL e = key[0];
    LL n = key[1];
    return modExponent(cyp, e, n);
}


bitset<56> getSynKey() {
    LL key = 0;
    for (int i = 0; i < 4; i++) {
        key = (key << 14) | (rand() & 0x3FFF);
    }
    return bitset<56> (key);
}

LL textHash(string text) {
    LL a = 639571;
    LL b = 8372571;
    LL n = 16775219;
    LL res = 0;
    for (int i = 0; i < text.size(); i++) {
        res = (((int)text[i] + res) * a + b) % n;
    };
    return res;
}



string bitsetToString(bitset<56> set) {
    string res;
    LL ch = set.to_ullong();
    for (int i = 0; i < 7; i++) {
        res += (char)(ch % 256);
        ch >>= 8;
    }
    return res;
}

string symEncodingBlockXOR(string block, bitset<56> key) {
    if (block.size() == 0) {
        return "";
    }
    return block ^ bitsetToString(key);
}

string symSimpleEncodingTextXOR(string text, bitset<56> key) {
    string res;
    int amountBlocks = text.size() / 7;
    for (int i = 0; i < amountBlocks; i++) {
        res += symEncodingBlockXOR(text.substr(i*7, 7), key);
    }
    res += symEncodingBlockXOR(text.substr(amountBlocks * 7, text.size() - amountBlocks * 7), key);
    return res;
}

string symGrappledEncodingTextXOR(string text, bitset<56> key, string initVal) {
    string res;
    string block;
    int amountBlocks = text.size() / 7;
    int rest = text.size() % 7;
    for (int i = 0; i < amountBlocks; i++) {
        if (i == 0) {
            block = text.substr(0, 7) ^ initVal;
        } else {
            block = text.substr(i*7, 7) ^ block;
        }
        block = symEncodingBlockXOR(block, key);
        res += block;
    }
    if (rest > 0) {
        block = text.substr(amountBlocks * 7, text.size() - amountBlocks * 7) ^ block;
        res += symEncodingBlockXOR(block, key);
    }
    return res;
}

string symGrappledDecodingTextXOR(string text, bitset<56> key, string initVal) {
    string res;
    string block;
    int amountBlocks = text.size() / 7;
    int rest = text.size() % 7;
    for (int i = 0; i < amountBlocks; i++) {
        block = symEncodingBlockXOR(text.substr(i*7, 7), key);
        if (i == 0) {
            block = block ^ initVal;
        } else {
            block = block ^ text.substr((i-1)*7, 7);
        }
        res += block;
    }
    if (rest > 0) {
        block = text.substr(amountBlocks * 7, text.size() - amountBlocks * 7);
        block = symEncodingBlockXOR(block, key);
        res += block ^ text.substr((amountBlocks - 1) * 7, 7);
    }
    return res;
}

void split(string s, vector<string> &v){
	string temp = "";
	for(int i=0;i<s.length();++i){
		if(s[i]=='\n'){
			v.push_back(temp);
			temp = "";
		}
		else {
			temp.push_back(s[i]);
		}
	}
	v.push_back(temp);
}

void block() {
		chmod(concat(em_path, "/emulator/syslog/input"), 0);
		chmod(concat(em_path, "/emulator/syslog/close"), 0);
		chmod(concat(em_path, "/emulator/syslog/open"), 0);
		chmod(concat(em_path, "/emulator/syslog/keys"), 0);
		chmod(concat(em_path, "/emulator/syslog"), 0);
		chmod(concat(em_path, "/emulator"), 0);
}

void unblock() {
		chmod(concat(em_path, "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
		chmod(concat(em_path, "/emulator/syslog"), S_IRWXU | S_IRWXG | S_IRWXO);
		chmod(concat(em_path, "/emulator/syslog/keys"), S_IRWXU | S_IRWXG | S_IRWXO);
		chmod(concat(em_path, "/emulator/syslog/input"), S_IRWXU | S_IRWXG | S_IRWXO);
		chmod(concat(em_path, "/emulator/syslog/close"), S_IRWXU | S_IRWXG | S_IRWXO);
		chmod(concat(em_path, "/emulator/syslog/open"), S_IRWXU | S_IRWXG | S_IRWXO);
}

char* readFile(char* filename) {
	char* res = new char[1000];
	strcpy(res, "");
	ifstream file(filename);
  string line;
  while(getline(file, line)) {
    strcat(res, line.c_str());
    strcat(res, "\n");
  }
	return res;
}

void writeFile(const char* text, char* filename) {
	FILE *fp = NULL;
  if ((fp = fopen(filename, "w")) == NULL) {
   	perror("error"); 
   	exit(1);
  }
  fprintf(fp, text);
  fclose(fp);
}

void changeKeys() {
		LL* keys = getEncryptionKeys(getPrimeNumber(ipow(2, 28)), getPrimeNumber(ipow(2, 28)));
		string init = "1234567";
    bitset<56> key = getSynKey();
		unblock();
		string text;
		text += to_string(time(NULL));
		text += "\n";
		text += to_string(keys[0]);
		text += "\n";
		text += to_string(keys[1]);
		text += "\n";
		text += to_string(keys[2]);
		text += "\n";
		text += init;
		text += "\n";
		text += key.to_string();
		writeFile(text.c_str(), concat(em_path, "/emulator/syslog/keys"));
		block();
}

void encodeSimple() {
		unblock();
		vector<string> lines;
		split(readFile(concat(em_path, "/emulator/syslog/keys")), lines);
		string text = readFile(concat(em_path, "/emulator/syslog/input"));
		bitset<56> symKey(lines[5]);
		
		char * endp;
		
		LL* privateKey = new LL[2] {strtoull(lines[2].c_str(), &endp, 10), strtoull(lines[3].c_str(), &endp, 10)};
		LL* publicKey = new LL[2] {strtoull(lines[1].c_str(), &endp, 10), strtoull(lines[3].c_str(), &endp, 10)};
		
		LL sign = encryptDecrypt(textHash(text), privateKey);
		string encodedText = symSimpleEncodingTextXOR(text, symKey);
		
		string outputText;
		outputText += to_string(sign);
		outputText += "\n";
		outputText += encodedText;
		writeFile(outputText.c_str(), concat(em_path, "/emulator/syslog/close"));
		
		block();
}

void decodeSimple() {
		unblock();
		vector<string> lines;
		vector<string> closeLines;
		split(readFile(concat(em_path, "/emulator/syslog/keys")), lines);
		split(readFile(concat(em_path, "/emulator/syslog/close")), closeLines);
		bitset<56> symKey(lines[5]);
		
		char * endp;
		
		LL* privateKey = new LL[2] {strtoull(lines[2].c_str(), &endp, 10), strtoull(lines[3].c_str(), &endp, 10)};
		LL* publicKey = new LL[2] {strtoull(lines[1].c_str(), &endp, 10), strtoull(lines[3].c_str(), &endp, 10)};
		
		string decodedText = symSimpleEncodingTextXOR(closeLines[1], symKey);
		LL hashDecodedText = textHash(decodedText);
		LL decodedSign = encryptDecrypt(strtoull(closeLines[0].c_str(), &endp, 10), publicKey);
		
		if (hashDecodedText == decodedSign) {
				writeFile(decodedText.c_str(), concat(em_path, "/emulator/syslog/open"));
		} else {
				cout << "Digital signature is not verified" << endl;
		}
		block();
}

void encodeGrappled() {
		unblock();
		vector<string> lines;
		split(readFile(concat(em_path, "/emulator/syslog/keys")), lines);
		string text = readFile(concat(em_path, "/emulator/syslog/input"));
		bitset<56> symKey(lines[5]);
		string initVal = lines[4];
		
		char * endp;
		
		LL* privateKey = new LL[2] {strtoull(lines[2].c_str(), &endp, 10), strtoull(lines[3].c_str(), &endp, 10)};
		LL* publicKey = new LL[2] {strtoull(lines[1].c_str(), &endp, 10), strtoull(lines[3].c_str(), &endp, 10)};
		
		LL sign = encryptDecrypt(textHash(text), privateKey);
		string encodedText = symGrappledEncodingTextXOR(text, symKey, initVal);
		
		string outputText;
		outputText += to_string(sign);
		outputText += "\n";
		outputText += encodedText;
		
		writeFile(outputText.c_str(), concat(em_path, "/emulator/syslog/close"));
		
		block();
}

void decodeGrappled() {
		unblock();
		vector<string> lines;
		vector<string> closeLines;
		split(readFile(concat(em_path, "/emulator/syslog/keys")), lines);
		split(readFile(concat(em_path, "/emulator/syslog/close")), closeLines);
		bitset<56> symKey(lines[5]);
		string initVal = lines[4];
		
		char * endp;
		
		LL* privateKey = new LL[2] {strtoull(lines[2].c_str(), &endp, 10), strtoull(lines[3].c_str(), &endp, 10)};
		LL* publicKey = new LL[2] {strtoull(lines[1].c_str(), &endp, 10), strtoull(lines[3].c_str(), &endp, 10)};
		
		string decodedText = symGrappledDecodingTextXOR(closeLines[1], symKey, initVal);
		
		LL hashDecodedText = textHash(decodedText);
		
		LL decodedSign = encryptDecrypt(strtoull(closeLines[0].c_str(), &endp, 10), publicKey);
		
		if (hashDecodedText == decodedSign) {
				writeFile(decodedText.c_str(), concat(em_path, "/emulator/syslog/open"));
		} else {
				cout << "Digital signature is not verified" << endl;
		}
		
		block();
}

void quit() {
	block();
	exit(0);
}

int main() {
		getcwd(em_path, 200);
		srand(time(NULL));
		unblock();
		
		vector<string> lines;
		split(readFile(concat(em_path, "/emulator/syslog/keys")), lines);
		time_t t = stoi(lines[0]);
		if(time(NULL) - t > TIME_INTERVAL) {
				cout << "Recomendation: You should change keys. For this you should input chkeys" << endl;
		}
		
		char input[100];
		while(true) {
			vector<string> words = {};
		
			cin.getline(input, 15);
		
			if (strcmp(input, "chkeys") == 0)
				changeKeys();
			else if (strcmp(input, "simenc") == 0)
				encodeSimple();
			else if (strcmp(input, "simdec") == 0)
				decodeSimple();
			else if (strcmp(input, "grapenc") == 0)
				encodeGrappled();
			else if (strcmp(input, "grapdec") == 0)
				decodeGrappled();
			else if (strcmp(input, "quit") == 0 || strcmp(input, "q") == 0)
				quit();
		}

    return 0;
}
