/*define colour control characters*/
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void shufti_benchmarks(int size, int loops, int M, bool has_match);
void rshufti_benchmarks(int size, int loops, int M, bool has_match);
void truffle_benchmarks(int size, int loops, int M, bool has_match);
void rtruffle_benchmarks(int size, int loops, int M, bool has_match);
void noodle_benchmarks(int size, int M, const char *lit_str, int lit_len, char nocase);