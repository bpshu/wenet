export GLOG_logtostderr=1
export GLOG_v=2
wav_path=/Github/wenet-20200416/wenet/runtime/server/x86/20210121_unified_transformer_server/BAC009S0764W0121.wav
wav_path=/Github/wenet-20200416/wenet/runtime/server/x86/wav/test.wav
nthreads=$1
outfile=$2
./build/websocket_client_threads_main \
    --host 127.0.0.1 \
    --port 10086 \
    --wav_path ${wav_path}   \
    --nthreads ${nthreads}  \
    >> ${outfile} 2>&1
   # 2>&1 | tee client.log




