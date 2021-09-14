#include "benchmarks.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <time.h>
#include <functional>
#include <vector>

#define MAX_LOOPS    500000000
#define MAX_MATCHES  10

int main(){
    std::function<void(int,int,int,bool)>  functions[] = { shufti_benchmarks, rshufti_benchmarks, truffle_benchmarks, rtruffle_benchmarks };
    int sizes[] =  {  16000,  32000,  64000, 120000, 1600000, 2000000, 2500000, 3500000, 150000000, 250000000, 350000000, 500000000 };
    const char charset[] = "aAaAaAaAAAaaaaAAAAaaaaAAAAAAaaaAAaaa"; 
    for (size_t i = 0; i < std::size(sizes); i++) {
        for(int j = 0; j < 4; j++) {
            functions[j](sizes[i], MAX_LOOPS / sizes[i], MAX_MATCHES, false);
            functions[j](sizes[i], MAX_LOOPS / sizes[i], MAX_MATCHES, true);  
        } 
    }
    for(size_t i=0; i < std::size(sizes); i++){
        //we imitate the noodle unit tests
        for (int char_len = 1; char_len < 9; char_len++) {
            char *str = new char[char_len];
            for (int j=0; j<char_len; j++) {
                srand (time(NULL));
                int key = rand() % + 36 ;
                str[char_len] = charset[key];
                str[char_len + 1] = '\0';
            }
            noodle_benchmarks(sizes[i],  MAX_LOOPS / sizes[i], str,char_len, 0);
            delete [] str;    
        }
    }
    return 0;
}