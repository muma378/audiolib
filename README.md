AUDIO
=================

### Intro

Audio is a C++ library  to provide easy access on WAV files. It now provides abilities described as below: 

- Gets header information;
- Downsampling;
- Slicing wav into segments in a specified length, (while no voice were to be truncated);
- Converts stereo to mono;
- Extracts a segment with given begining and ending.

### Install

We provide a quick way to install the audio lib on POSIX system, just to run:
```bash
./script/swig.sh install 
```
to remove installed libs, you need to do:
```bash
./sciript/swig.sh clean
```

Besides, if you don't want to pollute your system environment, via entering:
```bash
python setup.py build_ext --inplace
```
entension module _audio.so and audio.py will be built at loacal directory.

### Usage

*declare*
```c_cpp
\#include "audio.h"
BaseWave wav;
```

*open*
```c_pp
try{
    wav.open(filename)
}catch(const UnreadableException& e){
    cerr << e.what() << endl;    
    exit(2);
}
```

*write*
```c_pp
wav.normalize();	// to clean header
wav.set_filename("audio.wav");
wav.write();	// or wav.write("audio.wav");
```

### Python Wrapped

Audio is now wrapped by Python via SWIG. All functions, methods and exceptions are able to be accessed through python calls. 
Noted before importing the module, it is necessary to generate wrapper files and compile them into dynamical library, which can be achieved by typing:
```bash
python setup.py install
```

It becomes pretty easy to call methods in python, for example:
```python
import audio
w = audio.BaseWave()
try:
	w.open('./error.wav')
except audio.UnreadableException, e:
	print e.what()

bwv = audio.BaseWaveVec()
b.smart_truncate(60*30, bwv)
for slice in bwv:
	slice.write()

```



### Note

In fact, waveform audio file format is quite flexible and unstrunctured (well, in some sense). Extra bytes and information may be contained in the header, which makes it pretty difficult to parse all info. To simplify processing, all extra bytes were **ignored** and the minimum subset of data were collected. Therefore, after processing, ```normalize()``` was ought to be called to generate a clean, suitable header.

Even though it works for most cases, to be honest, the processing is quite rough. Therefore, the class is not recommended to use for people who wishes to keep other infomation. The alternative way is inheriting the base class and implementing your own ```open``` and ```write``` methods.

A documentation - [*Multimedia Programming Interface and Data Specifications 1.pdf*](http://www.tactilemedia.com/info/MCI_Control_Info.html) describing all specification is contained in current directory, *page 65~76* focuses on waveform audio file format.

### TODOs

The library is about to provide more common functions in the future:

1. *voice segment*: split wav into snippets to make each of which contains an unbroken semantic voice;
1. *strip*: remove the begining and ending whose energy was below a threshold;

Happy Writing!
