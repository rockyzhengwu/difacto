#include "difacto/predicter.h"
#include "./predict/fm_predicter.h"
namespace difacto {

DMLC_REGISTER_PARAMETER(FMPredicterParam);

Predicter* Predicter::Create(const std::string& type ) {
  Predicter* predicter = nullptr;
  if (type == "fm") {
    predicter = new FMPredicter();
  } else if (type == "logit") {
    //
  } else if (type == "logit_delta") {
    //
  } else {
    LOG(FATAL) << "unknown loss type";
  }
  return predicter;
}

KWArgs Predicter::Init(const KWArgs& kwargs){
  std::cout << "predict parent init function \n";
  return kwargs;
}

}  // namespace difacto