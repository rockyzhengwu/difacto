#ifndef DIFACTO_PREDICT_FM_PREDICTOR_H_
#define DIFACTO_PREDICT_FM_PREDICTOR_H_
#include "dmlc/data.h"
#include "difacto/loss.h"
#include "difacto/predicter.h"

#include <iostream>


namespace difacto {

  struct FMPredicterParam : public dmlc::Parameter<FMPredicterParam> {
  /**
   * \brief the embedding dimension
   */
  std::string model_in;
  std::string data_in;
  std::string data_out;

  DMLC_DECLARE_PARAMETER(FMPredicterParam) {
    DMLC_DECLARE_FIELD(model_in).set_default("fm.model");
    DMLC_DECLARE_FIELD(data_in);
    DMLC_DECLARE_FIELD(data_out).set_default("/tmp/difacto_fm_predict_output.txt");
  }
};

class FMPredicter: public Predicter{
  public:
    FMPredicter(){}
    virtual ~FMPredicter(){}

    KWArgs Init(const KWArgs& kwargs){
      // create loss and init
      std::cout << "fm predict init\n";
      auto  remain = param_.InitAllowUnknown(kwargs);
      std::cout << remain.size() << "\n";
      return kwargs;
    }

    void PredictCli(){
      // read data and predict
      std::cout << "predict cli\n";
    }
    
  private: 
    FMPredicterParam param_;
};

}
#endif
