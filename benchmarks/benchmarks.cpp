#include "benchmarks.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <time.h>
#include <functional>

int main(){
    int sizes[]=     {  16000,  32000,  64000, 120000, 1600000, 2000000, 2500000, 3500000, 150000000, 250000000, 350000000, 500000000};
    int f_loops[]=   {  70000,  50000,  30000,  10000,    1000,    1000,    1000,    1000,         7,         7,         5,         3};
    int t_loops[]=   { 200000, 150000, 100000,  70000,    5000,    5000,    5000,    5000,        50,        50,        50,        50};
    int exp_len[]=   {     15,     15,     15,     15,       5,       5,       5,       5,         5,         5,         5,         5};
    int nd_loops[]=  { 250000, 150000, 100000, 100000,   10000,     1000,     1000,     1000,      100,       100,      100,        100};
    const char charset[] = "aAaAaAaAAAaaaaAAAAaaaaAAAAAAaaaAAaaa";
    std::string labels[] = {"\x1B[33m Benchmarks(kbytes)  \x1B[0m\n", "\x1B[33m Benchmarks(kbytes)  \x1B[0m\n",
                            "\x1B[33m Benchmarks(kbytes) \x1B[0m\n", "\x1B[33m  Benchmarks(kbytes) \x1B[0m\n", 
                            "\x1B[33m Benchmarks(Mbytes)  \x1B[0m\n", "\x1B[33m Benchmarks(Mbytes)   \x1B[0m\n",
                            "\x1B[33m Benchmarks(Mbytes) \x1B[0m\n", "\x1B[33m  Benchmarks(Mbytes) \x1B[0m\n",
                            "\x1B[33m Benchmarks(Gbytes)  \x1B[0m\n", "\x1B[33m Benchmarks(Gbytes)   \x1B[0m\n",
                            "\x1B[33m Benchmarks(Gbytes) \x1B[0m\n", "\x1B[33m  Benchmarks(Gbytes) \x1B[0m\n"
                        };
    
    std::function<void(int,int,int,bool)>  functions[] = { shufti_benchmarks, rshufti_benchmarks, truffle_benchmarks, rtruffle_benchmarks };
    for (int i=11; i<12; i++) {
        std::cout << labels[i];
        for(int j=0; j<4; j++){
            functions[j](sizes[i],f_loops[i],exp_len[i],false);
            functions[j](sizes[i],t_loops[i],exp_len[i],true);  
        } 
    }
    for(int i=0; i<12; i++){
        if(i==0){
            std::cout<<std::endl <<"\x1B[33m noodle Benchmarks(kbytes) \x1B[0m"<<std::endl;
        }else if (i==4)
        {
            std::cout<<std::endl <<"\x1B[33m noodle Benchmarks(Mbytes) \x1B[0m"<<std::endl;
        }else if (i==8)
        {
            std::cout<<std::endl <<"\x1B[33m noodle Benchmarks(Gbytes) \x1B[0m"<<std::endl;
        }
        for (int char_len = 1; char_len < 9; char_len++) {
            char *str = new char[char_len];
            for (int j=0; j<char_len; j++) {
                srand (time(NULL));
                int key = rand() % + 36 ;
                str[char_len] = charset[key];
                str[char_len + 1] = '\0';
            }
            noodle_benchmarks(sizes[i], nd_loops[i], str,char_len,0);
            delete [] str;    
        }
    }
    return 0;
}