#include "fakePrint.h"

using namespace std;
size_t Print::print(const String &val) { cout << val.c_str(); return 0; }
size_t Print::print(const char val[]) { cout << val; return 0; }
size_t Print::print(char val) { cout << val; return 0; }
size_t Print::print(unsigned char val, int base) { cout << val; return 0; }
size_t Print::print(int val, int base) { cout << val; return 0; }
size_t Print::print(unsigned int val, int base) { cout << val; return 0; }
size_t Print::print(long val, int base) { cout << val; return 0; }
size_t Print::print(unsigned long val, int base) { cout << val; return 0; }
size_t Print::print(long long val, int base) { cout << val; return 0; }
size_t Print::print(unsigned long long val, int base) { cout << val; return 0; }
size_t Print::print(double val, int base) { cout << val; return 0; }

size_t Print::println(const String &val) { cout << val.c_str() << "\n"; return 0; }
size_t Print::println(const char val[]) { cout << val << "\n"; return 0; }
size_t Print::println(char val) { cout << val << "\n"; return 0; }
size_t Print::println(unsigned char val, int base) { cout << val << "\n"; return 0; }
size_t Print::println(int val, int base) { cout << val << "\n"; return 0; }
size_t Print::println(unsigned int val, int base) { cout << val << "\n"; return 0; }
size_t Print::println(long val, int base) { cout << val << "\n"; return 0; }
size_t Print::println(unsigned long val, int base) { cout << val << "\n"; return 0; }
size_t Print::println(long long val, int base) { cout << val << "\n"; return 0; }
size_t Print::println(unsigned long long val, int base) { cout << val << "\n"; return 0; }
size_t Print::println(double val, int base) { cout << val << "\n"; return 0; }
size_t Print::println() { cout << "\n"; return 0; }
