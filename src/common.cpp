
#include <vector>
#include <string>
#include <iostream>
#include "common.h"

using namespace std;

// test if the operation system was as same as our development environment
void validate_typesize() {
    if (sizeof(size8_t) == 1 &&
        sizeof(size16_t) == 2 &&
        sizeof(size32_t) == 4){
        return;
    }
    else{
        std::cerr << "critical error: types' size of the platform in use are not as same as the macros defined\n"
        << "please get the source code, alter it and then rebuild." << std::endl;
        exit(1);
    }
}