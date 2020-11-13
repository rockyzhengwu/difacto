#ifndef DIFACTO_PREDICTER_H_
#define DIFACTO_PREDICTER_H_
#include <string.h>
#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include "dmlc/io.h"
#include "./base.h"
#include "./loss.h"

namespace difacto {

class Predicter {
 public:
  static Predicter* Create(const std::string& type);

  Predicter() { }
  virtual ~Predicter() { }

  virtual KWArgs Init(const KWArgs& kwargs);
  void Predict(const std::string& input);
  virtual void Run(){
    // todo 读取
    std::cout << "run runction \n";
    PredictCli();
  }

 protected:
  virtual void PredictCli()=0;
  Loss* loss_;
};

}  // namespace difacto
#endif  // DIFACTO_PREDICTER_H_