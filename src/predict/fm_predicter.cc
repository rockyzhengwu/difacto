#include "./fm_predicter.h"
#include "difacto/base.h"
#include "data/row_block.h"
#include "data/localizer.h"
#include "difacto/store.h"
#include <fstream>

namespace difacto
{

void sigmoid(SArray<real_t>& in, SArray<real_t>& out) {
  CHECK_EQ(in.size(), out.size());
  for (size_t i = 0; i < in.size(); ++i) {
      out[i] = 1.0 / (1.0 + exp(-in[i]));
    }
  }

  KWArgs FMPredicter::Init(const KWArgs &kwargs)
  {
    std::cout << "fm init function \n";
    auto remain = Predicter::Init(kwargs);
    remain = param_.InitAllowUnknown(remain);
    updater_ = std::shared_ptr<SGDUpdater>(new SGDUpdater());
    updater_->Init(remain);
    // dmlc::Stream *ifs = CHECK_NOTNULL(dmlc::Stream::Create(param_.model_in.c_str(), "r"));
    updater_->LoadTxt(param_.model_in);

    // todo move nthread to config file
    loss_ = Loss::Create("fm", 2);
    remain = loss_->Init(remain);
    return kwargs;
  }

void FMPredicter::GetPos(const SArray<int>& len,
                        SArray<int>* w_pos, SArray<int>* V_pos) {
  size_t n = len.size();
  w_pos->resize(n);
  V_pos->resize(n);
  int* w = w_pos->data();
  int* V = V_pos->data();
  int p = 0;
  for (size_t i = 0; i < n; ++i) {
    int l = len[i];
    w[i] = l == 0 ? -1 : p;
    V[i] = l > 1 ? p+1 : -1;
    p += l;
  }
}

  void FMPredicter::PredictCli(){
    std::cout << "predict cli \n" ;
    std::cout << param_.data_val << "\n";
    Reader* reader =  new Reader(param_.data_val,
                        "libsvm",
                        0,
                        1,
                        256*1024*1024);
    // std::cout << reader->Next() << " next\n";
    std::vector<real_t> preds;
    std::ofstream outfile;

  
    outfile.open ("output.txt");
    while(reader->Next()){
      dmlc::RowBlock<feaid_t> blk = reader->Value();
      SArray<real_t> pred(blk.size);
      std::cout << blk.size << " data size \n";
      SArray<int> w_pos, V_pos;
      auto data = new dmlc::data::RowBlockContainer<unsigned>();
      auto feaids = std::make_shared<std::vector<feaid_t>>();
      Localizer lc(-1, 1); 
      lc.Compact(blk, data, feaids.get(), nullptr);


      auto values = new SArray<real_t>();
      auto lengths = new SArray<int>();
      SArray<feaid_t> sfeaids = SArray<feaid_t>(feaids);
      updater_->Get(sfeaids, Store::kWeight, values, lengths);
      GetPos(*lengths, &w_pos, &V_pos);
     
      std::vector<SArray<char>> inputs = {
            SArray<char>(*values), SArray<char>(w_pos), SArray<char>(V_pos)};
      std::chrono::high_resolution_clock::time_point begin;
      std::chrono::milliseconds duration;
      begin = std::chrono::high_resolution_clock::now();

      loss_->Predict(data->GetBlock(), inputs, &pred);
      duration = std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::high_resolution_clock::now()-begin);
      std::cout << "create ffm cost " << duration.count() << "ms\n";
      sigmoid(pred, pred);
      int i=0;
      for(real_t &p: pred){
        outfile<< p << " "  << blk.label[i] << "\n";
        ++i;
      }
    }
  
  }
} // namespace difcto
