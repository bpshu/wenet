export GLOG_logtostderr=1
export GLOG_v=2
model_dir=/wenet/runtime/server/x86/20210121_unified_transformer_server

./build/websocket_server_main \
    --port 10086 \
    --chunk_size 16 \
    --model_path ${model_dir}/final.zip \
    --dict_path ${model_dir}/words.txt \
    2>&1 | tee server.log

# a=`cat server.log | grep "websocket_server\.cc\:101" | awk '{if($NF < 1000){sum += $NF; n += 1}};END{print sum}'`
# b=`cat server.log | grep "websocket_server\.cc\:111" | awk '{if($NF < 1000){sum += $NF; n += 1}};END{print sum}'`
c=`cat server.log | grep "websocket_server\.cc\:89" | awk '{if($NF < 1000){sum += $NF; n += 1}};END{print sum/n}'`

# echo $a $b $c
echo $c

