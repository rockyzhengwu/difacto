#ifndef DIFACTO_PREDICT_FM_PREDICTOR_H_
#define DIFACTO_PREDICT_FM_PREDICTOR_H_

#include "dmlc/data.h"
#include "difacto/loss.h"
#include "difacto/predicter.h"
#include "reader/reader.h"
#include "reader/batch_reader.h"
#include "difacto/updater.h"
#include "sgd/sgd_updater.h"

#include <memory>
#include <iostream>


namespace difacto {

  struct FMPredicterParam : public dmlc::Parameter<FMPredicterParam> {
  /**
   * \brief the embedding dimension
   */
  std::string model_in;
  std::string data_val;
  std::string data_out;

  DMLC_DECLARE_PARAMETER(FMPredicterParam) {
    DMLC_DECLARE_FIELD(model_in).set_default("fm.model");
    DMLC_DECLARE_FIELD(data_val);
    DMLC_DECLARE_FIELD(data_out).set_default("/tmp/difacto_fm_predict_output.txt");
  }
};

class FMPredicter: public Predicter{
  public:
    FMPredicter(){}
    virtual ~FMPredicter(){}

    KWArgs Init(const KWArgs& kwargs) override;
     
    void PredictCli() override;
    
  private: 
    FMPredicterParam param_;
    std::shared_ptr<Updater> updater_;
    
};

}
#endif
