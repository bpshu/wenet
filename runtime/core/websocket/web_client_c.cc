#include <memory>
#include "websocket/web_client_c.h"
#include "websocket/websocket_client.h"
#include "frontend/wav.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

void* SendStartSignal(const char* host, int port){
  // wenet::WebSocketClient client(FLAGS_host, FLAGS_port);
  // client.SendStartSignal();
  std::string host_ = host;
  std::unique_ptr<wenet::WebSocketClient> client = std::make_unique<wenet::WebSocketClient>(host_, port);
  client->SendStartSignal();
  return (void *) client.release();
}

void SendBinaryData(void* web_client, const void* data, size_t size){
  wenet::WebSocketClient* client = (wenet::WebSocketClient*) web_client;
  return client->SendBinaryData(data, size);
}

const char* GetNbestText(void* web_client){
  wenet::WebSocketClient* client = (wenet::WebSocketClient*) web_client;
  return client->GetNbestText();
}

void SendEndSignal(void* web_client){
  wenet::WebSocketClient* client = (wenet::WebSocketClient*) web_client;
  return client->SendEndSignal();
}

void ClientJoin(void* web_client){
  wenet::WebSocketClient* client = (wenet::WebSocketClient*) web_client;
  return client->Join();
}

void wavpath_read(const char* FLAGS_wav_path_, int* wav_sample_rate, float** wav_data, int* wav_num_sample){
  std::string FLAGS_wav_path = FLAGS_wav_path_;
  wenet::WavReader wav_reader(FLAGS_wav_path);
  const int sample_rate = 16000;
  // Only support 16K
  *wav_sample_rate = wav_reader.sample_rate();
  // CHECK_EQ(*wav_sample_rate, sample_rate);
  *wav_num_sample = wav_reader.num_sample();
  // std::vector<float> pcm_data(wav_reader.data(),
  //                             wav_reader.data() + num_sample);
  // Send data every 0.5 second
  *wav_data = new float[*wav_num_sample];
  for(int i=0; i<*wav_num_sample; i++){
    (*wav_data)[i] = wav_reader.data()[i];
  }
  return;
}