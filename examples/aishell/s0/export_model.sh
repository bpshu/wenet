

. ./path.sh || exit 1;

dir=exp/20210203_unified_conformer_exp

python wenet/bin/export_jit.py \
        --config $dir/train.yaml \
        --checkpoint $dir/final.pt \
        --output_file $dir/final.zip
