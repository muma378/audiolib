//  exceptions.h
//  audio
//
//  Created by Xiao Yang on 16/4/8.
//  Copyright (c) 2016年 Young. All rights reserved.
//

#ifndef audio_exceptions_h
#define audio_exceptions_h
#include <string>
#include <cstring>


class BaseException
{
private:
    const char* message = "";
    
public:
    BaseException(){};
    BaseException(const char* msg);
    
    virtual const char* what() const throw();
    
};


class UnreadableException: public BaseException{
private:
    const char* message = "Unable to read the audio file";

public:
    using BaseException::BaseException;
    using BaseException::what;
};


class UnwritableException: public BaseException{
private:
    const char* message = "Unable to write the audio file";
    
public:
    using BaseException::BaseException;
    using BaseException::what;
};


class InvalidOperation: public BaseException {
private:
    const char* message = "operation is invalid";

public:
    using BaseException::BaseException;
    using BaseException::what;
};
#endif
