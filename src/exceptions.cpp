#include "exceptions.h"

BaseException::BaseException(const char* msg){
	this->message = msg;
}

const char* BaseException::what() const throw(){
	return message;
}
