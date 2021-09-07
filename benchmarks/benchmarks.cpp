#include "benchmarks.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <time.h>
#include <functional>
int main(){
    int sizes[]=   { 16000, 32000, 64000, 120000, 1600000, 2000000, 2500000, 3500000, 150000000, 250000000, 350000000, 500000000};
    int loops[]=   {  6000,  6000,  6000,   6000,    1000,    1000,    1000,    1000,        50,        50,        25,        25};
    int exp_len[]= {  2000,  2000,  2000,   2000,     250,     250,     250,     250,        10,        10,         5,         5};
    const char charset[] = "aAaAaAaAAAaaaaAAAAaaaaAAAAAAaaaAAaaa";
    std::string labels[] = {"\x1B[33m shuftiExec Benchmarks(kbytes)  \x1B[0m\n", "\x1B[33m rshuftiExec Benchmarks(kbytes)  \x1B[0m\n",
                            "\x1B[33m truffleExec Benchmarks(kbytes) \x1B[0m\n", "\x1B[33m rtruffleExec Benchmarks(kbytes) \x1B[0m\n", 
                            "\x1B[33m shuftiExec Benchmarks(Mbytes)  \x1B[0m\n", "\x1B[33m rhuftiExec Benchmarks(Mbytes)   \x1B[0m\n",
                            "\x1B[33m truffleExec Benchmarks(Mbytes) \x1B[0m\n", "\x1B[33m rtruffleExec Benchmarks(Mbytes) \x1B[0m\n",
                            "\x1B[33m shuftiExec Benchmarks(Gbytes)  \x1B[0m\n", "\x1B[33m rhuftiExec Benchmarks(Gbytes)   \x1B[0m\n",
                            "\x1B[33m truffleExec Benchmarks(Gbytes) \x1B[0m\n", "\x1B[33m rtruffleExec Benchmarks(Gbytes) \x1B[0m\n"
                        };
    std::function<void(int,int,int,bool)>  functions[] = { shufti_benchmarks, rshufti_benchmarks, truffle_benchmarks, rtruffle_benchmarks };
    for (int i=0; i<12; i++) {
        std::cout << labels[i];
        for(int j=0; j<4; j++){
            functions[j](sizes[i],loops[i],exp_len[i],false);
            functions[j](sizes[i],loops[i],exp_len[i],true);  
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
            noodle_benchmarks(sizes[i], loops[i], str,char_len,0);
            delete [] str;    
        }
    }
    return 0;
}