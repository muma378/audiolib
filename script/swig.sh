#!/bin/bash

if [ $# = 0 ]
then
	CFLAGS='-fPIC -std=c++11 -c'
	swig -c++ -python -o audio_wrap.cpp  audio.i
	g++ $CFLAGS audio.cpp common.cpp exceptions.cpp
	g++ $CFLAGS audio_wrap.cpp -I/usr/include/python2.7/
	g++ -shared audio.o common.o exceptions.o audio_wrap.o -o _audio.so
elif [ $1 = 'clean' ]
then
	rm -rf build
	find ./ | grep -E '\.o$|\.so$|^audio\.py|^audio_wrap\.cpp' | xargs rm
elif [ $1 = 'install' ]
then
    python setup.py build_ext --inplace
    PYTHON_SITEPACKAGES=`python -c "import site;print site.getusersitepackages()"`
    if [ ! -e ${PYTHON_SITEPACKAGES} ];then
        echo "user's site-packages is not existed, creating it now..."
        echo $PYTHON_SITEPACKAGES
        sudo mkdir -p $PYTHON_SITEPACKAGES 
    fi
    
    echo "moving _audio.so --> "$PYTHON_SITEPACKAGES
    sudo cp ./_audio.so $PYTHON_SITEPACKAGES
    echo "moving audio.py --> "$PYTHON_SITEPACKAGES
    sudo cp ./swig/audio.py $PYTHON_SITEPACKAGES

fi
