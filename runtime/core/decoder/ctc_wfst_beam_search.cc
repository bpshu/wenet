// Copyright 2021 Mobvoi Inc. All Rights Reserved.
// Author: binbinzhang@mobvoi.com (Binbin Zhang)

#include "decoder/ctc_wfst_beam_search.h"

namespace wenet {

void DecodableTensorScaled::Reset() {
  num_frames_ready_ = 0;
  done_ = false;
  // Give an empty initialization, will throw error when
  // AcceptLoglikes is not called
  logp_ = torch::zeros({1}); 
}

void DecodableTensorScaled::AcceptLoglikes(const torch::Tensor& logp) {
  CHECK_EQ(logp.dim(), 1);
  ++num_frames_ready_;
  // TODO(Binbin Zhang): Avoid copy here
  logp_ = logp;
  // LOG(INFO) << logp.sizes() << " " <<logp.dim();
  accessor_.reset(new torch::TensorAccessor<float, 1>(
      logp_.data_ptr<float>(), logp_.sizes().data(), logp_.strides().data()));
  //
  // LOG(INFO) << accessor_->size(0) << " " << accessor_->size(1);
  LogLikelihoodSorted();
}

float DecodableTensorScaled::LogLikelihood(int32 frame, int32 index) {
  CHECK(accessor_ != nullptr);
  CHECK_GT(index, 0);
  CHECK_LE(index, logp_.size(0));
  CHECK_LT(frame, num_frames_ready_);
  // LOG(INFO) << accessor_->size(0);
  return scale_ * (*accessor_)[index - 1];
}

struct node{
  int id;
  float val;
};

bool cmp(node a, node b){
  return a.val > b.val;
}

bool DecodableTensorScaled::LogLikelihoodSorted() {
  struct node LikelihoodSorted[accessor_->size(0)];
  for(int i=0; i<accessor_->size(0); i++){
    LikelihoodSorted[i].id = i;
    LikelihoodSorted[i].val = (*accessor_)[i];
  }
  // for(int i=0; i<10; i++){
  //   LOG(INFO) << LikelihoodSorted[i].id << " " << LikelihoodSorted[i].val;
  // }
  std::sort(LikelihoodSorted, LikelihoodSorted+accessor_->size(0), cmp);
  // for(int i=0; i<10; i++){
  //   LOG(INFO) << LikelihoodSorted[i].id << " " << LikelihoodSorted[i].val;
  // }
  // LOG(INFO) << num_frames_ready_; 
  std::vector<int> temp(accessor_->size(0));
  token_ids_.resize(num_frames_ready_, temp);

  for(int i=0; i<accessor_->size(0); i++){
    token_ids_[num_frames_ready_-1][LikelihoodSorted[i].id] = i;
    // std::cout << num_frames_ready_-1 << " " << LikelihoodSorted[i].id <<" "<< LikelihoodSorted[i].val<<" "<<i<<" end ";
  }
  // std::cout << std::endl;
  // exit(0);
}

bool DecodableTensorScaled::IsLastFrame(int32 frame) const {
  CHECK_LT(frame, num_frames_ready_);
  return done_ && (frame == num_frames_ready_ - 1);
}

int32 DecodableTensorScaled::NumIndices() const {
  LOG(FATAL) << "Not implement";
  return 0;
}

CtcWfstBeamSearch::CtcWfstBeamSearch(const fst::Fst<fst::StdArc>& fst,
                                     const CtcWfstBeamSearchOptions& opts)
    : decodable_(opts.acoustic_scale), decoder_(fst, opts), opts_(opts) {
  Reset();
}

CtcWfstBeamSearch::CtcWfstBeamSearch(const fst::Fst<fst::StdArc>& fst,
                                     const fst::Fst<fst::StdArc>& fst_kw,
                                     const std::vector<int>& keywords_ids,
                                     const CtcWfstBeamSearchOptions& opts)
    : decodable_(opts.acoustic_scale), decoder_(fst, fst_kw, keywords_ids, opts), opts_(opts) {
  
  Reset();
}

void CtcWfstBeamSearch::Reset() {
  num_frames_ = 0;
  decoded_frames_mapping_.clear();
  is_last_frame_blank_ = false;
  inputs_.clear();
  outputs_.clear();
  likelihood_.clear();
  times_.clear();
  decodable_.Reset();
  decoder_.InitDecoding();
}

void CtcWfstBeamSearch::Search(const torch::Tensor& logp) {
  CHECK_EQ(logp.dtype(), torch::kFloat);
  CHECK_EQ(logp.dim(), 2);
  if (0 == logp.size(0)) {
    return;
  }
  // LOG(INFO) << logp.dim() << " "  << logp.size(0) <<" " <<logp.size(1);
  // Every time we get the log posterior, we decode it all before return
  auto accessor = logp.accessor<float, 2>();
  for (int i = 0; i < logp.size(0); i++) {
    // LOG(INFO) << "logp.size(0) " <<logp.size(0) <<" logp.size(1) " <<logp.size(1);
    float blank_score = std::exp(accessor[i][0]);
    // LOG(INFO) << "blank_score " << blank_score << " opts_.blank_skip_thresh " << opts_.blank_skip_thresh;
    if (blank_score > opts_.blank_skip_thresh) {
      VLOG(3) << "skipping frame " << num_frames_ << " score " << blank_score;
      // LOG(INFO) << "skipping frame " << num_frames_ << " score " << blank_score;
      is_last_frame_blank_ = true;
      last_frame_prob_ = logp[i];
    } else {
      // LOG(INFO) << "AdvanceDecoding";
      if (is_last_frame_blank_) {
        decodable_.AcceptLoglikes(last_frame_prob_);
        decoder_.AdvanceDecoding(&decodable_, 1);
      }
      decodable_.AcceptLoglikes(logp[i]);
      decoder_.AdvanceDecoding(&decodable_, 1);
      decoded_frames_mapping_.push_back(num_frames_);
      is_last_frame_blank_ = false;
    }
    num_frames_++;
  }
  // Get the best path
  inputs_.clear();
  outputs_.clear();
  likelihood_.clear();
  if (decoded_frames_mapping_.size() > 0) {
    inputs_.resize(1);
    outputs_.resize(1);
    likelihood_.resize(1);
    kaldi::Lattice lat;
    decoder_.GetBestPath(&lat, false);
    std::vector<int> alignment;
    kaldi::LatticeWeight weight;
    fst::GetLinearSymbolSequence(lat, &alignment, &outputs_[0], &weight);
    ConvertToInputs(alignment, &inputs_[0]);
    VLOG(3) << weight.Value1() << " " << weight.Value2();
    // LOG(INFO) << "skipping frame " << num_frames_ << " score " << blank_score;
    likelihood_[0] = -weight.Value2();
  }
}

void CtcWfstBeamSearch::FinalizeSearch() {
  decodable_.SetFinish();
  decoder_.FinalizeDecoding();
  inputs_.clear();
  outputs_.clear();
  likelihood_.clear();
  if (decoded_frames_mapping_.size() > 0) {
    // Get N-best path by lattice(CompactLattice)
    kaldi::CompactLattice clat;
    decoder_.GetLattice(&clat, true);
    kaldi::Lattice lat, nbest_lat;
    fst::ConvertLattice(clat, &lat);
    // TODO(Binbin Zhang): it's n-best word lists here, not character n-best
    fst::ShortestPath(lat, &nbest_lat, opts_.nbest);
    std::vector<kaldi::Lattice> nbest_lats;
    fst::ConvertNbestToVector(nbest_lat, &nbest_lats);
    int nbest = nbest_lats.size();
    inputs_.resize(nbest);
    outputs_.resize(nbest);
    likelihood_.resize(nbest);
    for (int i = 0; i < nbest; i++) {
      kaldi::LatticeWeight weight;
      std::vector<int> alignment;
      fst::GetLinearSymbolSequence(nbest_lats[i], &alignment, &outputs_[i],
                                   &weight);
      std::cout << "path-id " << i<<" : ";
      for(auto x: alignment)
        std::cout << x << " ";
      std::cout << std::endl;
      ConvertToInputs(alignment, &inputs_[i]);
      likelihood_[i] = -weight.Value2();
    }
  }
}

void CtcWfstBeamSearch::ConvertToInputs(const std::vector<int>& alignment,
                                        std::vector<int>* input) {
  input->clear();
  for (size_t i = 0; i < alignment.size(); i++) {
    if (alignment[i] - 1 > 0) {
      input->push_back(alignment[i] - 1);
    }
  }
}

}  // namespace wenet
