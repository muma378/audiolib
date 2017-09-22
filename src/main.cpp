//
//  main.cpp
//  audio
//	exit with critical error (for system, operation and etc) return 1, 
//  with error happened in reading returns 2, error occured in writting returns 3
//  Created by Xiao Yang on 16/4/8.
//  Copyright (c) 2016年 Young. All rights reserved.
//


#include "audio.h"
#include "common.h"
#include "exceptions.h"

using namespace std;

int main(int argc, const char * argv[]) {

    validate_typesize();
	BaseWave wav = BaseWave();
	if (argc != 4){
		cout << "number of arguments is incorrect\n";
		cout << "usage: " << argv[0] << " src_file dst_file" << endl;
		return 1;
	}
	const char* src_file = argv[1];
	const char* dst_file = argv[2];
    const char* online_file = argv[3];
    
	try{
		wav.open(src_file);
        //cout << wav << endl;
	}
	catch (const UnreadableException& e){	
		cerr << e.what() << endl;;
		exit(2);
	}
    
    if (wav.is_stereo()) {
        BaseWave mono = wav.stereo2mono();
        mono.set_filename(dst_file);
        wav = mono;
    }
    
    if(!wav.is_normalized()){
        wav.normalize();
    }
    
	vector<BaseWave*> wav_vec;
	wav.set_filename(dst_file);		// alter the filename to renew the place to save
	try{
		wav.balanced_truncate(5*60, wav_vec, 3*60);		// cause smart_truncate will generate files in the same directory
	}
	catch (const UnreadableException& e){
		cerr << e.what() << endl;
		exit(3);
	}

	try{
		uint counter = 0;
		for (vector<BaseWave*>::iterator it = wav_vec.begin(); it != wav_vec.end(); it++) {
			(*it)->write();
            (*it)->downsample(8000);
			(*it)->set_filename(online_file);
			if (wav_vec.size() > 1) {
				const char * online_name = (*it)->get_clip_name(counter++);
				(*it)->set_filename(online_name);
			}
            (*it)->write();
		}
	}
	catch (const UnwritableException& e){
		cerr << e.what() << endl;
		exit(3);
	}

    return 0;
}
