#!/bin/bash

stage=0 # start from 0 if you need to start from data preparation
stop_stage=5

. ./path.sh || exit 1;
. tools/parse_options.sh || exit 1;

if [ ${stage} -le 0 ] && [ ${stop_stage} -ge 0 ]; then
    echo "processing tal_cn_en"
    dir="/data2/multiData/TAL/tal_cn_en"
    local_dir="data/tmp/tal_cn_en"
    mkdir -p ${local_dir}
    mkdir -p ${local_dir}/original
    python local/tal_cn_en_pre.py ${dir}/label ${local_dir}/original/wenet.text.1
    cat ${local_dir}/original/wenet.text.1 | \
        awk '{print $1 " /data2/multiData/TAL/tal_cn_en/cs_wav/" $1".wav"}' > ${local_dir}/original/wav.scp.1
    #用local/file_exists.py已经验证wav.scp中的音频文件都存在，这里不在写了。
    cat ${local_dir}/original/wenet.text.1 | awk '{print $1}' > ${local_dir}/original/utt.1
    cat ${local_dir}/original/utt.1 | awk -F'_' '{print $4}' > ${local_dir}/original/spk
    #
    paste -d'-' ${local_dir}/original/spk ${local_dir}/original/utt.1 > ${local_dir}/original/utt
    paste -d'-' ${local_dir}/original/spk ${local_dir}/original/wenet.text.1  > ${local_dir}/original/text
    paste -d'-' ${local_dir}/original/spk ${local_dir}/original/wav.scp.1 > ${local_dir}/original/wav.scp
    #
    paste -d' ' ${local_dir}/original/utt ${local_dir}/original/spk | sort > ${local_dir}/original/utt2spk
    perl ./utils/utt2spk_to_spk2utt.pl ${local_dir}/original/utt2spk > ${local_dir}/original/spk2utt
    #
    ./utils/subset_data_dir_tr_cv.sh --cv_spk_percent 20 ${local_dir}/original ${local_dir}/train ${local_dir}/dev_test
    ./utils/subset_data_dir_tr_cv.sh --cv_spk_percent 50 ${local_dir}/dev_test ${local_dir}/dev ${local_dir}/test
    #
    ./utils/fix_data_dir.sh  ${local_dir}/train  
    ./utils/fix_data_dir.sh  ${local_dir}/dev
    ./utils/fix_data_dir.sh  ${local_dir}/test
fi

if [ ${stage} -le 1 ] && [ ${stop_stage} -ge 1 ]; then
    ori_dir=/data3/002.data3.beiping.shu/Github/leyan-workspace/AM_train/multi_cn/s1/data
    for x in aishell aidatatang magicdata thchs;do
        echo "processing ", ${x}
        local_dir="data/tmp/"${x}
        for y in train dev test;do
            mkdir -p ${local_dir}/${y}
            python local/file_exists.py ${ori_dir}/${x}/${y}/wav.scp ${local_dir}/${y}/wav.scp
            cp ${ori_dir}/${x}/${y}/utt2spk ${local_dir}/${y}/utt2spk
            cp ${ori_dir}/${x}/${y}/spk2utt ${local_dir}/${y}/spk2utt
            python local/delete_space.py ${ori_dir}/${x}/${y}/text ${local_dir}/${y}/text
            ./utils/fix_data_dir.sh  ${local_dir}/${y}
        done        
    done
fi

if [ ${stage} -le 2 ] && [ ${stop_stage} -ge 2 ]; then
    ori_dir=/data3/002.data3.beiping.shu/Github/leyan-workspace/AM_train/multi_cn/s1/data
    for x in primewords stcmds;do
        echo "processing ", ${x}
        local_dir="data/tmp/"${x}
        ./utils/subset_data_dir_tr_cv.sh --cv_spk_percent 20 ${ori_dir}/${x}/train ${local_dir}/train ${local_dir}/dev_test
        ./utils/subset_data_dir_tr_cv.sh --cv_spk_percent 50 ${local_dir}/dev_test ${local_dir}/dev ${local_dir}/test
        for y in train dev test;do
            python local/delete_space.py ${local_dir}/${y}/text ${local_dir}/${y}/text.1
            mv ${local_dir}/${y}/text.1 ${local_dir}/${y}/text
            ./utils/fix_data_dir.sh  ${local_dir}/${y}
        done
    done
fi

if [ ${stage} -le 3 ] && [ ${stop_stage} -ge 3 ]; then
    ori_dir=/data3/002.data3.beiping.shu/Github/leyan-workspace/AM_train/multi_cn/s1/data
    for x in tal;do
        echo "processing ", ${x}
        local_dir="data/tmp/"${x}
        ./utils/subset_data_dir_tr_cv.sh --cv_spk_percent 20 ${ori_dir}/${x} ${local_dir}/train ${local_dir}/dev_test
        ./utils/subset_data_dir_tr_cv.sh --cv_spk_percent 50 ${local_dir}/dev_test ${local_dir}/dev ${local_dir}/test
        for y in train dev test;do
            python local/delete_space.py ${local_dir}/${y}/text ${local_dir}/${y}/text.1
            mv ${local_dir}/${y}/text.1 ${local_dir}/${y}/text
            ./utils/fix_data_dir.sh  ${local_dir}/${y}
        done
    done
fi


if [ ${stage} -le 4 ] && [ ${stop_stage} -ge 4 ]; then
    ori_dir=/data3/002.data3.beiping.shu/Github/leyan-workspace/AM_train/multi_cn/s1/data
    for x in leyan_v2;do
        echo "processing ", ${x}
        local_dir="data/tmp/"${x}
        ./utils/subset_data_dir_tr_cv.sh --cv_spk_percent 20 ${ori_dir}/${x}/train ${local_dir}/train ${local_dir}/dev
        mkdir -p ${local_dir}/test
        cp ${ori_dir}/${x}/test/wav.scp ${local_dir}/test/wav.scp
        python local/delete_space.py ${ori_dir}/${x}/test/text ${local_dir}/test/text
        cp ${ori_dir}/${x}/test/utt2spk ${local_dir}/test/utt2spk
        cp ${ori_dir}/${x}/test/spk2utt ${local_dir}/test/spk2utt
        for y in train dev test;do
            sed -i 's/data1\/leyanData/data3\/001.data1.leyanData/g' ${local_dir}/${y}/wav.scp
            sed -i 's/data2\/leyanData/data3\/001.data2.leyanData/g' ${local_dir}/${y}/wav.scp
            ./utils/fix_data_dir.sh  ${local_dir}/${y}
        done
    done
fi

if [ ${stage} -le 5 ] && [ ${stop_stage} -ge 5 ]; then
    ori_dir=/data2/multiData/AISHELL/aishell-3
    echo "processing aishell3; first version"
    local_dir="data/tmp/aishell3"
    for x in train test;do
        mkdir -p ${local_dir}/${x}
        find  ${ori_dir}/${x} -name "*.wav" > ${local_dir}/${x}/wav.list
        cat ${local_dir}/${x}/wav.list | awk -F'/' '{print $NF}' | sed 's/\.wav//' > ${local_dir}/${x}/utt
        paste -d' ' ${local_dir}/${x}/utt ${local_dir}/${x}/wav.list > ${local_dir}/${x}/wav.scp
        cat ${local_dir}/${x}/wav.list | awk -F'/' '{print $(NF-1)}' > ${local_dir}/${x}/spk
        paste -d' ' ${local_dir}/${x}/utt ${local_dir}/${x}/spk > ${local_dir}/${x}/utt2spk
        perl ./utils/utt2spk_to_spk2utt.pl ${local_dir}/${x}/utt2spk > ${local_dir}/${x}/spk2utt
        python local/aishell3_text_pre.py /data2/multiData/AISHELL/aishell-3/${x}/content.txt ${local_dir}/${x}/text han
        python local/aishell3_text_pre.py /data2/multiData/AISHELL/aishell-3/${x}/content.txt ${local_dir}/${x}/text.pinyin pinyin
        sed -i 's/\.wav//g' ${local_dir}/${x}/text
        sed -i 's/\.wav//g' ${local_dir}/${x}/text.pinyin
    done
fi

if [ ${stage} -le 6 ] && [ ${stop_stage} -ge 6 ]; then
    ori_dir=/data2/multiData/AISHELL/aishell-3
    echo "processing aishell3; second version"
    local_dir="data/tmp/aishell3"
    mkdir -p ${local_dir}_v2
    ./utils/subset_data_dir_tr_cv.sh --cv_spk_percent 18 ${local_dir}/train ${local_dir}_v2/train ${local_dir}_v2/dev
    cp -r ${local_dir}/test ${local_dir}_v2/
    for x in train dev test;do
        ./utils/fix_data_dir.sh ${local_dir}_v2/${x}
    done
fi

if [ ${stage} -le 7 ] && [ ${stop_stage} -ge 7 ]; then
    echo "combine data"
    for x in train dev test;do
        #
        ./local/combine_data.sh data/${x} \
            data/tmp/aidatatang/${x} \
            data/tmp/aishell/${x} \
            data/tmp/aishell3_v2/${x} \
            data/tmp/leyan_v2/${x} \
            data/tmp/magicdata/${x} \
            data/tmp/primewords/${x} \
            data/tmp/stcmds/${x} \
            data/tmp/tal/${x} \
            data/tmp/tal_cn_en/${x} \
            data/tmp/thchs/${x}
    done
fi
