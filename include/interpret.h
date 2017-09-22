//
//  interpret.h
//  
//
//  Created by Xiao Yang on 16/4/15.
//
//

#ifndef audio_interpret_h
#define audio_interpret_h

#include <iostream>
#include "common.h"

template <class T>
class Interpreter {
    T* content;
    
public:
    Interpreter();
};


class BaseInterpreter {

protected:
    char* content;
    uint size;
    
public:
    virtual void say(){
        std::cout << "base class" << std::endl;
    }
};



#endif /* defined(____interpret__) */
