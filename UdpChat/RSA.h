#pragma once

#include <string>

class RSA
{
	inline static unsigned long long limitPrimary = 500;
	inline static unsigned long long limit = 10000;

public:

	struct Key {
		int k;
		int n;
	};

	Key GetOpenKey() {
		return Key(e, n);
	}

	Key GetCloseKey() {
		return Key(d, n);
	}

	static bool CheckPrimary(int n);

	RSA();

	std::vector<int> RSA_Encrypt(std::vector<int> str);

	std::vector<int> RSA_Decrypt(std::vector<int> str);

	unsigned int GetPrimary(int i);

	void RSAInitializeByOpenKey(Key key);

	void RSAInitializeByCloseKey(Key key);

	void RSAInitializeBy(Key openKey, Key closeKey);

	void RSAInitializeByPrimary(int p = 0, int q = 0);

private:

	int e = -1, d = -1, n = -1;

	int modular(int base, unsigned int exp, unsigned int mod);

	int modInverse(int a, int m);

	int lcm(int a, int b);

	bool Test();

};

