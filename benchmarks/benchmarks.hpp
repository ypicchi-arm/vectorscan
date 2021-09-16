#include <functional>

/*define colour control characters*/
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


void noodle_benchmarks(int size, int M, const char *lit_str, int lit_len, char nocase);
void run_benchmarks(int size, int loops, int M, bool has_match, std::function <const u8 *(m128, m128, const u8 *, const u8 *)> function);