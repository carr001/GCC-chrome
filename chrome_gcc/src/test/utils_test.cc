#include <iostream>
#include "utils.h"

void utils_test1(){
    int a = 2000000000;
    std::cout<<number_to_fix_len_string(a,20)<<std::endl;
}
void utils_test2(){
    int a = 2000;
    std::cout<<number_to_fix_len_string(a,10)<<std::endl;
}
int main(int argc, char* argv[]){
    utils_test1();
    utils_test2();
    // std::cout<<"test end"<<std::endl;
};

