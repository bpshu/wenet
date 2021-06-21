path='/data/beiping.shu/Github/wenet/examples/aishell/s0'
./build/decoder_main \
    --rescoring_weight 1.0 \
    --ctc_weight 0.5 \
    --chunk_size -1 \
    --wav_scp exp/wav.scp \
    --model_path ${path}/exp/20210203_unified_conformer_exp/final.zip \
    --dict_path ${path}/data/lang_test/words.txt \
    --token_path ${path}/data/lang_test/tokens.txt \
    --keywords_path ${path}/data/local/lm_kw/text \
    --fst_path ${path}/data/lang_test/TLG.fst \
    --fst_kw_path ${path}/data/lang_test_kw/TLG.fst \
    --beam 15.0 \
    --lattice_beam 7.5 \
    --max_active 7000 \
    --acoustic_scale 1.0 \
    --blank_skip_thresh 0.98 \
    --result log/res