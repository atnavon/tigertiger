#pragma once

#include "common.h"

#include <xaudio2.h>

#define BGM_MAX 4
#define SE_MAX 19
#define SE_PLAY_MAX 64

typedef struct sound_t
{
	int16* data;
	uint32 size;
	uint8 channel_num;
	uint32 sample_rate;
	uint32 sample_num;
	uint32 loop_sample_start;
	uint32 loop_sample_num;
} sound_t;

typedef struct sound_sys_t
{
	IXAudio2* xaudio2;
	IXAudio2MasteringVoice* mastering_voice;
	sound_t bgm[BGM_MAX];
	sound_t se[SE_MAX];
	IXAudio2SourceVoice* bgm_voice;
	IXAudio2SourceVoice* se_voices[SE_PLAY_MAX];
} sound_sys_t;

void sound_init(sound_sys_t* sound_sys);
void sound_fini(sound_sys_t* sound_sys);
void sound_update(sound_sys_t* sound_sys);
void sound_bgm_load(sound_sys_t* sound_sys, uint8 idx, const char* path);
void sound_bgm_play(sound_sys_t* sound_sys, uint8 idx);
void sound_bgm_stop(sound_sys_t* sound_sys);
void sound_bgm_pitch(sound_sys_t* sound_sys, float pitch);
void sound_se_load(sound_sys_t* sound_sys, const char* path);
void sound_se_play(sound_sys_t* sound_sys, uint8 idx);
uint8 sound_load(sound_sys_t* sound_sys, sound_t* sound, const void* data);
void sound_free(sound_sys_t* sound_sys, sound_t* sound);
IXAudio2SourceVoice* sound_play(sound_sys_t* sound_sys, const sound_t* sound);
