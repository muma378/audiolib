#!/usr/bin/env python

"""
setup.py for audio processing library
"""
import os
from distutils.core import setup, Extension


audio_module = Extension('_audio', 
                         sources=['swig/audio.i', 'src/audio.cpp', 'src/common.cpp', 'src/exceptions.cpp'],
                         swig_opts=['-c++', '-I./include', '-outcurrentdir'],
                         extra_compile_args=['-std=c++11', ],
                         include_dirs=['include',],
                         language = 'c++',
                         )

# os.environ["CC"] = "g++"
setup (name = 'audio',
        version = '1.0',
        author = 'Yang',
        author_email = "xiaoyang0117@gmail.com",
        description = "provides simple interfaces to process audio",
        ext_modules = [audio_module],
        py_modules = ["audio"],
        )
