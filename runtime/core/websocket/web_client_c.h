#ifndef web_client_c_h
#define web_client_c_h

#include <stddef.h>
#ifdef __cplusplus
extern "C"{
#endif

extern void* SendStartSignal(const char* host, int port);

extern void SendBinaryData(void* web_client, const void* data, size_t size);

extern void SendEndSignal(void* web_client);

extern void ClientJoin(void* web_client);

extern const char* GetNbestText(void* web_client);

extern void wavpath_read(const char* FLAGS_wav_path_, int* wav_sample_rate, float** wav_data, int* wav_num_sample);

#ifdef __cplusplus
}
#endif
#endif /* speech_eval_h */