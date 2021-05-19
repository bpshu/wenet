// Copyright (c) 2020 Mobvoi Inc (Binbin Zhang)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
//
#include <iostream>
#include <memory>
#include <string>
#include <thread>
//
#include "gflags/gflags.h"
#include "glog/logging.h"
// #include "frontend/wav.h"
// #include "websocket/websocket_client.h"
#include "websocket/web_client_c.h"

DEFINE_string(host, "127.0.0.1", "host of websocket server");
DEFINE_int32(port, 10086, "port of websocket server");
DEFINE_string(wav_path, "", "test wav file path");

int main(int argc, char *argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, false);
  google::InitGoogleLogging(argv[0]);
  void* web_client = SendStartSignal(FLAGS_host.c_str(), FLAGS_port);
  int wav_sample_rate = 0;
  int wav_num_sample = 0;
  float* wav_data = NULL;
  wavpath_read(FLAGS_wav_path.c_str(), &wav_sample_rate, &wav_data, &wav_num_sample);
  const int sample_rate = 16000;
  // Only support 16K
  // CHECK_EQ(wav_sample_rate, sample_rate);
  const int num_sample = wav_num_sample;
  std::vector<float> pcm_data(wav_data, wav_data + wav_num_sample);
  if(wav_data){
    delete[] wav_data;
  }

  // Send data every 0.5 second
  const float interval = 0.005;
  const int sample_interval = interval * sample_rate;
  for (int start = 0; start < wav_num_sample; start += sample_interval) {
    int end = std::min(start + sample_interval, wav_num_sample);
    // Convert to short
    std::vector<int16_t> data;
    data.reserve(end - start);
    for (int j = start; j < end; j++) {
      data.push_back(static_cast<int16_t>(pcm_data[j]));
    }
    // TODO(Binbin Zhang): Network order?
    // Send PCM data
    const void* ptr = static_cast<void*>(data.data());
    SendBinaryData(web_client, ptr, data.size() * sizeof(int16_t));
    VLOG(2) << "Send " << data.size() << " samples";
    std::this_thread::sleep_for(
        std::chrono::milliseconds(static_cast<int>(interval * 1000)));
  }
  
  auto start = std::chrono::steady_clock::now();
  SendEndSignal(web_client);
  ClientJoin(web_client);
  std::string res(GetNbestText(web_client));
  VLOG(2) << "res: " << res;
  if(web_client){
    delete web_client;
  }
  auto end = std::chrono::steady_clock::now();
  VLOG(2) << "Total latency: "
          << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                 .count()
          << "ms.";
  return 0;
}
