#include <random>
#include <numeric>
#include "RSA.h"
#include "Utils.h"


bool RSA::CheckPrimary(int n) {

	if (n < 2)
		return false;

	for (int i = 2; i <= sqrt(n); i++) {
		if (n % i == 0) {
			return false;
		}
	}
	return true;
}

RSA::RSA() {
}

unsigned int RSA::GetPrimary(int i) {
	std::vector<unsigned int> prime;
	prime.reserve(i + 10);
	unsigned int* bitmap = (unsigned int*)calloc(limit / 64 + ((limit & 63) ? 1 : 0), sizeof(unsigned int));
	prime.push_back(2);
	prime.push_back(3);
	unsigned int max_prime = 3;
	bool need_fill = true;
	while (need_fill) {
		unsigned int step = max_prime << 1;
		for (unsigned int i = max_prime * max_prime; i < limit; i += step) { // Вычеркиваем кратные max_pr
			bitmap[i >> 6] |= (1 << ((i >> 1) & 31));
		}
		if (max_prime * max_prime >= limit) need_fill = false;
		for (unsigned int i = max_prime + 2; i < limit; i += 2) {
			if ((bitmap[i >> 6] & (1 << ((i >> 1) & 31))) == 0) {
				prime.push_back(i);
				if (i - 1 < prime.size())
					return prime[i - 1];
				if (need_fill) {
					max_prime = i;
					break;
				}
			}
		}
	}
	free(bitmap);
	std::random_device rd;
	std::mt19937 mersenne(rd());

	int temp = (prime.size() > limitPrimary) ? limitPrimary : prime.size();

	return prime[mersenne() % temp];
}

void RSA::RSAInitializeByOpenKey(RSA::Key key) {
	e = key.k;
	n = key.n;
}

void RSA::RSAInitializeByCloseKey(RSA::Key key) {
	d = key.k;
	n = key.n;
}

void RSA::RSAInitializeBy(RSA::Key openKey, RSA::Key closeKey) {
	if (openKey.n != closeKey.n)
		throw std::exception("Open key and close key unsuitable!");

	e = openKey.k;
	d = closeKey.k;
	n = closeKey.n;
}

void RSA::RSAInitializeByPrimary(int p, int q) {

	bool defaults = (p == 0 && q == 0);
	int times = 0;
	do
	{
		int _p = p; 
		int _q = q;
		if (defaults)
		{
			std::random_device rd;
			std::mt19937 mersenne(rd());
			_p = GetPrimary(mersenne() % limitPrimary);
			_q = GetPrimary(mersenne() % limitPrimary);
			times = 0;
		}
		if (!CheckPrimary(_p) || !CheckPrimary(_q))
			throw std::exception("p and q not primary!");

		n = _p * _q;

		if (n > 4000 && defaults)
			continue;

		int phi = (_p - 1) * (_q - 1);
		int lambda = lcm(_p - 1, _q - 1);
		std::vector<int> tot;
		for (int i = 3; i < lambda; i++)
		{

			if (std::gcd(i, lambda) == 1) {
				tot.push_back(i);

			}
		}
		if (times == tot.size() && defaults)
			continue;
		else if (times == tot.size())
			throw std::exception("bad primary number!");

		e = tot[times++];

		d = modInverse(e, lambda);
		if (d != e && Test()) {
			break;
		}
	} while (true);
}

std::vector<int> RSA::RSA_Encrypt(std::vector<int> str)
{
	std::vector<int> encrypt;
	encrypt.resize(str.size());
	for (int i = 0; i < str.size(); i++)
		encrypt[i] = modular(str[i], e, n);

	return encrypt;
}


std::vector<int> RSA::RSA_Decrypt(std::vector<int> str)
{
	std::vector<int> decrypted;
	decrypted.resize(str.size());
	for (int i = 0; i < str.size(); i++)
		decrypted[i] = modular(str[i], d, n);

	return decrypted;
}



int RSA::modular(int base, unsigned int exp, unsigned int mod)
{
	long res = 1;
	while (exp) {
		if (exp & 1) {
			res *= base;
			res %= mod;
		}
		base *= (base % mod);
		base %= mod;
		exp >>= 1;
	}
	return res % mod;
}

int RSA::modInverse(int a, int m)
{
	int m0 = m;
	int y = 0, x = 1;

	if (m == 1)
		return 0;

	while (a > 1)
	{

		int q = a / m;
		int t = m;


		m = a % m, a = t;
		t = y;


		y = x - q * y;
		x = t;
	}


	if (x < 0)
		x += m0;

	return x;
}



int RSA::lcm(int a, int b)
{
	return (a * b) / std::gcd(a, b);
}


bool RSA::Test() {
	std::string text = "qwertyuiop[]asdfghjkl;'zxcvbnm,./1234567890-={}:\"<>?!";

	auto t = RSA_Encrypt(Convert(text));

	auto l = Convert(RSA_Decrypt(t));

	return text.compare(l) == 0;
}