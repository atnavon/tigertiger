#include "sound.h"

#include <opus.h>

#include "file.h"
#include "sar1.h"

#pragma comment(lib, "opus.lib")

#define SADF_MAGIC 'fdas'
#define SADF_FMT_OPUS 'supo'
#define SADF_FMT_ADPCM 'mcpd'

enum sadf_flag
{
	SADF_FLAG_LOOP = 0x2,
};

typedef struct sadf_t
{
	uint32 magic;
	uint32 version;
	uint32 fmt;
	uint32 _pad1;
	uint32 header_fmt;
	uint32 header_size;
	uint8 channel_num;
	uint8 flags;
	uint32 data_offs;
	uint32 data_size;
	uint32 sample_rate;
	uint32 sample_num;
	uint32 loop_start_sample;
	uint32 loop_end_sample;
	uint32 adpcm_data_size;
	uint32 stream_type;
	uint32 _pad2;
	uint32 _pad3;
	uint32 frame_size;
	uint32 loop_start_frame;
	uint32 loop_end_frame;
	uint32 frame_num;
	uint32 frame_table_offs;
	uint32 frame_table_size;
	uint32 _pad4;
	uint32 adpcm_channel_offs[8];
} sadf_t;

typedef struct xsp_t
{
	float vol_dist_near;
	float vol_near;
	float vol_dist_far;
	float vol_far;
	uint16 prio;
	uint16 id;
	uint32 pitch;
	uint32 pitch_rand;
	float vol_rand;
	uint32 unit;
} xsp_t;

#define ADPCM_FRAME_SIZE 8
#define ADPCM_FRAME_SAMPLE_NUM 14

typedef struct adpcm_param_t
{
	int16 coef[16];
} adpcm_param_t;

typedef struct adpcm_context_t
{
	int16 pred_scale;
	int16 hist[2];
} adpcm_context_t;

typedef struct adpcm_header_t
{
	__declspec(align(64)) adpcm_param_t param;
	__declspec(align(64)) adpcm_context_t ctx;
	uint32 sample_num;
	uint32 sample_rate;
	uint8 loop;
	uint32 loop_start_sample;
	uint32 loop_end_sample;
} adpcm_header_t;

typedef struct adpcm_frame_t
{
	uint8 scale : 4;
	uint8 pred : 4;
	int8 sample1 : 4;
	int8 sample0 : 4;
	int8 sample3 : 4;
	int8 sample2 : 4;
	int8 sample5 : 4;
	int8 sample4 : 4;
	int8 sample7 : 4;
	int8 sample6 : 4;
	int8 sample9 : 4;
	int8 sample8 : 4;
	int8 sample11 : 4;
	int8 sample10 : 4;
	int8 sample13 : 4;
	int8 sample12 : 4;
} adpcm_frame_t;

void sound_init(sound_sys_t* sound_sys)
{
	memset(sound_sys, 0, sizeof(sound_sys_t));

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	XAudio2Create(&sound_sys->xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	sound_sys->xaudio2->lpVtbl->CreateMasteringVoice(sound_sys->xaudio2, &sound_sys->mastering_voice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, NULL, NULL, AudioCategory_GameEffects);

#if DEBUG
	XAUDIO2_DEBUG_CONFIGURATION debug_conf = { 0 };

	debug_conf.TraceMask = XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_ERRORS;
	debug_conf.BreakMask = XAUDIO2_LOG_ERRORS;
	debug_conf.LogThreadID = TRUE;
	debug_conf.LogFileline = TRUE;
	debug_conf.LogFunctionName = TRUE;
	debug_conf.LogTiming = FALSE;

	sound_sys->xaudio2->lpVtbl->SetDebugConfiguration(sound_sys->xaudio2, &debug_conf, NULL);
#endif
}

void sound_fini(sound_sys_t* sound_sys)
{
	if (sound_sys->bgm_voice)
	{
		sound_sys->bgm_voice->lpVtbl->DestroyVoice(sound_sys->bgm_voice);
	}

	for (uint8 i = 0; i < SE_PLAY_MAX; ++i)
	{
		if (sound_sys->se_voices[i])
		{
			sound_sys->se_voices[i]->lpVtbl->DestroyVoice(sound_sys->se_voices[i]);
		}
	}

	sound_sys->xaudio2->lpVtbl->Release(sound_sys->xaudio2);

	for (uint8 i = 0; i < BGM_MAX; ++i)
	{
		sound_free(sound_sys, &sound_sys->bgm[i]);
	}

	for (uint8 i = 0; i < SE_MAX; ++i)
	{
		sound_free(sound_sys, &sound_sys->se[i]);
	}

	memset(sound_sys, 0, sizeof(sound_sys_t));

	CoUninitialize();
}

void sound_update(sound_sys_t* sound_sys)
{
	for (uint8 i = 0; i < SE_PLAY_MAX; ++i)
	{
		IXAudio2SourceVoice* src_voice = sound_sys->se_voices[i];

		if (src_voice)
		{
			XAUDIO2_VOICE_STATE state;

			src_voice->lpVtbl->GetState(src_voice, &state, 0);

			if (!state.BuffersQueued)
			{
				src_voice->lpVtbl->DestroyVoice(src_voice);

				sound_sys->se_voices[i] = NULL;
			}
		}
	}
}

void sound_bgm_load(sound_sys_t* sound_sys, uint8 idx, const char* path)
{
	void* data = file_read(path, NULL);

	if (!data)
	{
		return;
	}

	sound_load(sound_sys, &sound_sys->bgm[idx], data);

	free(data);
}

void sound_bgm_play(sound_sys_t* sound_sys, uint8 idx)
{
	sound_bgm_stop(sound_sys);

	sound_sys->bgm_voice = sound_play(sound_sys, &sound_sys->bgm[idx]);
}

void sound_bgm_stop(sound_sys_t* sound_sys)
{
	if (!sound_sys->bgm_voice)
	{
		return;
	}

	sound_sys->bgm_voice->lpVtbl->Stop(sound_sys->bgm_voice, 0, XAUDIO2_COMMIT_NOW);
	sound_sys->bgm_voice->lpVtbl->DestroyVoice(sound_sys->bgm_voice);

	sound_sys->bgm_voice = NULL;
}

void sound_bgm_pitch(sound_sys_t* sound_sys, float pitch)
{
	if (!sound_sys->bgm_voice)
	{
		return;
	}

	sound_sys->bgm_voice->lpVtbl->SetFrequencyRatio(sound_sys->bgm_voice, pitch, XAUDIO2_COMMIT_NOW);
}

void sound_se_load(sound_sys_t* sound_sys, const char* path)
{
	void* data = file_read(path, NULL);

	if (!data)
	{
		return;
	}

	sar1_t sar1;

	sar1_init(&sar1, data);

	uint32 file_num = sar1_file_count(&sar1);

	for (uint32 i = 0; i < file_num; ++i)
	{
		const xsp_t* xsp = sar1_file_meta(&sar1, i);

		sound_load(sound_sys, &sound_sys->se[xsp->id - 1], sar1_file_addr(&sar1, i));
	}

	sar1_fini(&sar1);

	free(data);
}

void sound_se_play(sound_sys_t* sound_sys, uint8 idx)
{
	uint8 play_idx = UINT8_MAX;

	for (uint8 i = 0; i < SE_PLAY_MAX; ++i)
	{
		if (!sound_sys->se_voices[i])
		{
			play_idx = i;

			break;
		}
	}

	if (play_idx == UINT8_MAX)
	{
		return;
	}

	sound_sys->se_voices[play_idx] = sound_play(sound_sys, &sound_sys->se[idx - 1]);
}

static void decode_opus(sound_t* sound, const sadf_t* sadf)
{
	const uint32* frame_table = MEMOFFS(sadf, sadf->frame_table_offs);

	sound->data = malloc(sadf->sample_num * sadf->channel_num * sizeof(int16));
	sound->size = 0;
	sound->sample_num = 0;
	sound->sample_rate = sadf->sample_rate;
	sound->channel_num = sadf->channel_num;

	OpusDecoder* decoder = opus_decoder_create(sadf->sample_rate, sound->channel_num, NULL);
	uint32 frame_num = sadf->frame_num;

	if (sadf->flags & SADF_FLAG_LOOP)
	{
		frame_num = MIN(frame_num, sadf->loop_end_frame + 1);

		sound->loop_sample_start = sadf->loop_start_sample;
		sound->loop_sample_num = sadf->loop_end_sample - sadf->loop_start_sample + 1;
	}

	for (uint32 i = 0; i < frame_num; ++i)
	{
		if (sound->sample_num >= sadf->sample_num)
		{
			break;
		}

		const void* frame = MEMOFFS(sadf, sadf->data_offs + frame_table[i]);
		uint32 frame_size = frame_table[i + 1] - frame_table[i];

		if (frame_size < 8)
		{
			break;
		}

		uint32 len = _byteswap_ulong(*(const uint32*)frame);

		if (len > frame_size)
		{
			break;
		}

		int frame_sample_num = opus_decode(decoder, MEMOFFS(frame, 8), len, MEMOFFS(sound->data, sound->size), sadf->sample_num - sound->sample_num, 0);

		if (frame_sample_num < 0)
		{
			break;
		}

		sound->sample_num += frame_sample_num;
		sound->size += frame_sample_num * sound->channel_num * sizeof(int16);
	}

	opus_decoder_destroy(decoder);
}

static void decode_adpcm(sound_t* sound, const sadf_t* sadf)
{
	sound->data = malloc(sadf->sample_num * sadf->channel_num * sizeof(int16));
	sound->size = 0;
	sound->sample_num = 0;
	sound->sample_rate = sadf->sample_rate;
	sound->channel_num = sadf->channel_num;

	const uint8* adpcm_data = MEMOFFS(sadf, sadf->data_offs);
	const adpcm_header_t* adpcm_headers = MEMOFFS(sadf, sizeof(sadf_t));
	adpcm_context_t ctx[2];

	for (uint32 i = 0; i < sound->channel_num; ++i)
	{
		ctx[i] = adpcm_headers[i].ctx;
	}

	uint32 remaining_sample_num = MIN(sadf->sample_num, adpcm_headers[0].sample_num);
	uint32 frame_idx = 0;

	while (remaining_sample_num)
	{
		uint32 frame_sample_num = MIN(remaining_sample_num, ADPCM_FRAME_SAMPLE_NUM);

		for (uint32 i = 0; i < sound->channel_num; ++i)
		{
			const adpcm_frame_t* frame = MEMOFFS(adpcm_data, sadf->adpcm_channel_offs[i] + frame_idx * ADPCM_FRAME_SIZE);
			int8 adpcm_samples[ADPCM_FRAME_SAMPLE_NUM];

			adpcm_samples[0] = frame->sample0;
			adpcm_samples[1] = frame->sample1;
			adpcm_samples[2] = frame->sample2;
			adpcm_samples[3] = frame->sample3;
			adpcm_samples[4] = frame->sample4;
			adpcm_samples[5] = frame->sample5;
			adpcm_samples[6] = frame->sample6;
			adpcm_samples[7] = frame->sample7;
			adpcm_samples[8] = frame->sample8;
			adpcm_samples[9] = frame->sample9;
			adpcm_samples[10] = frame->sample10;
			adpcm_samples[11] = frame->sample11;
			adpcm_samples[12] = frame->sample12;
			adpcm_samples[13] = frame->sample13;

			int32 scale = (1 << frame->scale) << 11;
			int16 coef0 = adpcm_headers[i].param.coef[frame->pred * 2];
			int16 coef1 = adpcm_headers[i].param.coef[frame->pred * 2 + 1];
			int16* out_pcm = MEMOFFS(sound->data, sound->size);

			for (uint32 j = 0; j < frame_sample_num; ++j)
			{
				int32 sample = (((adpcm_samples[j] * scale) + (coef0 * ctx[i].hist[0] + coef1 * ctx[i].hist[1])) + 1024) >> 11;

				sample = CLAMP(sample, INT16_MIN, INT16_MAX);

				ctx[i].hist[1] = ctx[i].hist[0];
				ctx[i].hist[0] = sample;

				out_pcm[j * sound->channel_num + i] = sample;
			}
		}

		remaining_sample_num -= frame_sample_num;
		++frame_idx;

		sound->sample_num += frame_sample_num;
		sound->size += frame_sample_num * sound->channel_num * sizeof(int16);
	}

	if (sadf->flags & SADF_FLAG_LOOP)
	{
		sound->loop_sample_start = sadf->loop_start_sample;
		sound->loop_sample_num = sadf->loop_end_sample - sadf->loop_start_sample + 1;
	}
}

uint8 sound_load(sound_sys_t* sound_sys, sound_t* sound, const void* data)
{
	memset(sound, 0, sizeof(sound_t));

	const sadf_t* sadf = data;

	if (!sadf || sadf->magic != SADF_MAGIC)
	{
		return 0;
	}

	switch (sadf->fmt)
	{
		case SADF_FMT_OPUS:
			decode_opus(sound, sadf);
			return 1;
		case SADF_FMT_ADPCM:
			decode_adpcm(sound, sadf);
			return 1;
	}

	return 0;
}

void sound_free(sound_sys_t* sound_sys, sound_t* sound)
{
	free(sound->data);

	memset(sound, 0, sizeof(sound_t));
}

IXAudio2SourceVoice* sound_play(sound_sys_t* sound_sys, const sound_t* sound)
{
	if (!sound->data)
	{
		return NULL;
	}

	WAVEFORMATEX wavefmt = { 0 };

	wavefmt.wFormatTag = WAVE_FORMAT_PCM;
	wavefmt.nChannels = sound->channel_num;
	wavefmt.nSamplesPerSec = sound->sample_rate;
	wavefmt.nAvgBytesPerSec = sound->sample_rate * sound->channel_num * sizeof(int16);
	wavefmt.nBlockAlign = sound->channel_num * sizeof(int16);
	wavefmt.wBitsPerSample = 16;

	IXAudio2SourceVoice* src_voice;

	if (FAILED(sound_sys->xaudio2->lpVtbl->CreateSourceVoice(sound_sys->xaudio2, &src_voice, &wavefmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL)))
	{
		return NULL;
	}

	float matrix[4];

	if (sound->channel_num == 1)
	{
		matrix[0] = 1.0f;
		matrix[1] = 1.0f;
	}
	else
	{
		matrix[0] = 1.0f;
		matrix[1] = 0.0f;
		matrix[2] = 0.0f;
		matrix[3] = 1.0f;
	}

	src_voice->lpVtbl->SetOutputMatrix(src_voice, NULL, sound->channel_num, 2, matrix, XAUDIO2_COMMIT_NOW);

	XAUDIO2_BUFFER audio_buf = { 0 };

	audio_buf.Flags = 0;
	audio_buf.AudioBytes = sound->size;
	audio_buf.pAudioData = sound->data;
	audio_buf.PlayBegin = 0;
	audio_buf.PlayLength = sound->sample_num;

	if (sound->loop_sample_num)
	{
		audio_buf.LoopBegin = sound->loop_sample_start;
		audio_buf.LoopLength = sound->loop_sample_num;
		audio_buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	audio_buf.pContext = NULL;

	src_voice->lpVtbl->SubmitSourceBuffer(src_voice, &audio_buf, NULL);
	src_voice->lpVtbl->Start(src_voice, 0, XAUDIO2_COMMIT_NOW);

	return src_voice;
}
