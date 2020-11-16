#include "./fm_predicter.h"
#include "difacto/base.h"
#include "data/row_block.h"

namespace difacto
{
  KWArgs FMPredicter::Init(const KWArgs &kwargs)
  {
    std::cout << "fm init function \n";
    auto remain = Predicter::Init(kwargs);
    remain = param_.InitAllowUnknown(remain);
    updater_ = std::shared_ptr<SGDUpdater>(new SGDUpdater());
    updater_->Init(remain);
    dmlc::Stream *ifs = CHECK_NOTNULL(dmlc::Stream::Create(param_.model_in.c_str(), "r"));
    updater_->Load(ifs, true);

    // todo move nthread to config file
    loss_ = Loss::Create("fm", 2);
    remain = loss_->Init(remain);
    std::cout << remain.size() << "\n";
    return kwargs;
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
    while(reader->Next()){
      dmlc::RowBlock<feaid_t> blk = reader->Value();
      std::cout << blk.size << "size \n";
      for(size_t i=0; i< blk.size;++i){
        dmlc::Row<feaid_t> row = blk[i];
        for(size_t j=0; j<row.length; ++j){
          std::cout << row.get_index(j) << ":" << row.get_value(j) << " " ;
        }
        std::cout << "\n";
        exit(0);
      }
      
    }
  }
} // namespace difcto
