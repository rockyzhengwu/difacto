/**
 * Copyright (c) 2015 by Contributors
 */
#include "difacto/updater.h"
#include "./sgd/sgd_updater.h"
#include "./bcd/bcd_updater.h"
namespace difacto {

DMLC_REGISTER_PARAMETER(SGDUpdaterParam);
void Updater::SaveAsTxt(const std::string& filename){
  
}
void Updater::LoadTxt(const std::string& filename){
  
}
}  // namespace difacto
