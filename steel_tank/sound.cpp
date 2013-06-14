#include "stdafx.h"
#include "sound.h"
#include "bass.h"

sound::sound(void){
	is_initialized = false;
}
sound::~sound(void){
	if(is_initialized){
		BASS_Free();
	}
}

bool sound::init(void){
	if (HIWORD(BASS_GetVersion())!=BASSVERSION) {
		return false;
	}

	if (!BASS_Init(-1,44100,0,NULL,NULL)){
		return false;
	}

	BASS_INFO bi={sizeof(bi)};
	BASS_GetInfo(&bi);
	if (bi.dsver<8) {
		BASS_Free();
		return false; //directx 8 not installed
	}

	is_initialized = true;
	return true;
}

void sound::playfile(string fspec){
	DWORD chan = 0;

	if(channels.size() > 50){
		chan = *channels.begin();
		channels.pop_front();
		BASS_MusicFree(chan);
		BASS_StreamFree(chan);
	}

	if (!(chan=BASS_StreamCreateFile(FALSE,fspec.data(),0,0,0))
		&& !(chan=BASS_MusicLoad(FALSE,fspec.data(),0,0,BASS_MUSIC_RAMP,0))) {
			return; // file not found?
	}

	BASS_ChannelPlay(chan,FALSE);

	channels.push_back(chan);
}
