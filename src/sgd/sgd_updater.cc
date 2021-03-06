/**
 * Copyright (c) 2015 by Contributors
 */
#include <string.h>
#include <math.h>
#include "./sgd_updater.h"
#include "difacto/store.h"
#include <fstream>
#include <sstream>


namespace difacto {

KWArgs SGDUpdater::Init(const KWArgs& kwargs) {
  return param_.InitAllowUnknown(kwargs);
}

void SGDUpdater::Evaluate(sgd::Progress* prog) const {
  real_t objv = 0;
  size_t nnz = 0;
  int dim = param_.V_dim;
  mu_.lock();
  for (const auto& it : model_) {
    const auto& e = it.second;
    if (e.w) ++nnz;
    objv += param_.l1 * fabs(e.w) + .5 * param_.l2 * e.w * e.w;
    if (e.V) {
      nnz += dim;
      for (int i = 0; i < dim; ++i) objv += .5 * param_.l2 * e.V[i] * e.V[i];
    }
  }
  mu_.unlock();
  prog->penalty = objv;
  prog->nnz_w = nnz;
}

void SGDUpdater::Get(const SArray<feaid_t>& fea_ids,
                     int val_type,
                     SArray<real_t>* weights,
                     SArray<int>* lens) {
  CHECK_EQ(val_type, Store::kWeight);
  int V_dim = param_.V_dim;
  size_t size = fea_ids.size();
  weights->resize(size * (1 + V_dim));
  lens->resize(V_dim == 0 ? 0 : size);
  int p = 0;
  for (size_t i = 0; i < size; ++i) {
    // ignore key not in model
    // if(model_.find(fea_ids[i])==model_.end()) continue;

    mu_.lock();
    auto& e = model_[fea_ids[i]];
    mu_.unlock();
    (*weights)[p++] = e.w;
    if (e.V) {
      memcpy(weights->data()+p, e.V, V_dim*sizeof(real_t));
      p += V_dim;
      (*lens)[i] = V_dim + 1;
    } else if (V_dim != 0) {
      (*lens)[i] = 1;
    }
  }
  weights->resize(p);
}

void SGDUpdater::Update(const SArray<feaid_t>& fea_ids,
                        int value_type,
                        const SArray<real_t>& values,
                        const SArray<int>& lens) {
  if (value_type == Store::kFeaCount) {
    CHECK_EQ(fea_ids.size(), values.size());
    for (size_t i = 0; i < fea_ids.size(); ++i) {
      mu_.lock();
      auto& e = model_[fea_ids[i]];
      mu_.unlock();
      e.fea_cnt += values[i];
      if (param_.V_dim > 0 && e.V == nullptr
          && e.w != 0 && e.fea_cnt > param_.V_threshold) {
        InitV(&e);
      }
    }
  } else if (value_type == Store::kGradient) {
    CHECK(has_aux_) << "no aux data";
    size_t size = fea_ids.size();
    bool w_only = lens.empty();
    if (w_only) {
      CHECK_EQ(values.size(), size);
    } else {
      CHECK_EQ(lens.size(), size);
    }
    int p = 0;
    real_t* v = values.data();
    for (size_t i = 0; i < size; ++i) {
      mu_.lock();
      auto& e = model_[fea_ids[i]];
      mu_.unlock();
      UpdateW(v[p++], &e);
      if (!w_only && lens[i] > 1) {
        CHECK_EQ(lens[i], param_.V_dim+1);
        CHECK(e.V != nullptr) << fea_ids[i];
        UpdateV(v+p, &e);
        p += param_.V_dim;
      }
    }
    CHECK_EQ(static_cast<size_t>(p), values.size());
  } else {
    LOG(FATAL) << ".....";
  }
}


void SGDUpdater::UpdateW(real_t gw, SGDEntry* e) {
  real_t sg = e->sqrt_g;
  real_t w = e->w;
  // update sqrt_g
  gw += w * param_.l2;
  e->sqrt_g = sqrt(sg * sg + gw * gw);
  // update z
  e->z -= gw - (e->sqrt_g - sg) / param_.lr * w;
  // update w by soft shrinkage
  real_t z = e->z;
  real_t l1 = param_.l1;
  if (z <= l1 && z >= - l1) {
    e->w = 0;
  } else {
    real_t eta = (param_.lr_beta + e->sqrt_g) / param_.lr;
    e->w = (z > 0 ? z - l1 : z + l1) / eta;
  }
  // update statistics
  if (w == 0 && e->w != 0) {
    if (param_.V_dim > 0 && e->V == nullptr && e->fea_cnt > param_.V_threshold) {
      InitV(e);
    }
  }
}

void SGDUpdater::UpdateV(real_t const* gV, SGDEntry* e) {
  int n = param_.V_dim;
  for (int i = 0; i < n; ++i) {
    real_t g = gV[i] + param_.V_l2 * e->V[i];
    real_t cg = e->V[i+n];
    e->V[i+n] = sqrt(cg * cg + g * g);
    float eta = param_.V_lr / (e->V[i+n] + param_.V_lr_beta);
    e->V[i] -= eta * g;
  }
}

void SGDUpdater::InitV(SGDEntry* e) {
  int n = param_.V_dim;
  e->V = new real_t[n*2];
  for (int i = 0; i < n; ++i) {
    e->V[i] = (rand_r(&param_.seed) / (real_t)RAND_MAX - 0.5) * param_.V_init_scale;
  }
  memset(e->V+n, 0, n*sizeof(real_t));
}

void SGDUpdater::SaveAsTxt(const std::string& filename){
  std::ofstream outf = std::ofstream(filename);
  int v_dim = param_.V_dim;
  for(const auto& it: model_){
    if(it.second.Empty()) continue;
    outf << (it.first) << ",";
    outf << it.second.w << ",";
    for(int i=0; i<v_dim; ++ i){
      if (i==v_dim-1){
        outf << it.second.V[i];
      }else {
        outf<< it.second.V[i] << ",";
      }
    }
    outf << "\n";
  }
  outf.close();
}

void SGDUpdater::LoadTxt(const std::string& filename) {
  std::ifstream inf = std::ifstream(filename);
  std::string line ;
  while(std::getline(inf, line)){
    std::vector<std::string> result;
    std::stringstream ss(line);
    while(ss.good()){
      std::string substr;
      std::getline( ss, substr, ',' );
      result.push_back( substr );
    }

    feaid_t feaid = std::atoi(result[0].c_str());
    SGDEntry* entry = new SGDEntry;
    entry->w = std::atof(result[1].c_str());
    entry->V = new real_t[param_.V_dim*2];
    std::cout << feaid << " ";
    std::cout << entry->w << " ";
    for(int i=0; i<param_.V_dim; ++i){
      entry->V[i] = std::atof(result[i+2].c_str());
      std::cout << entry->V[i] << " ";
    }
    std::cout << "\n";
    std::memset(entry->V+param_.V_dim, 0, param_.V_dim*sizeof(real_t));
    model_[feaid] = *entry;
    // model_.insert(std::pair<feaid_t, SGDEntry>(feaid, entry));
  }
  std::cout << model_[1].V[0]<< " " << model_[1].V[1]<< "\n";
  SaveAsTxt("fm.model.txt");
}

void SGDUpdater::Save(bool save_aux, dmlc::Stream *fo) const{
int saved = 0;
  for (const auto& it : model_) {
    if (it.second.Empty()) continue;
    int n = param_.V_dim;
    int v_dim = n;
    fo->Write(&it.first, sizeof(feaid_t));
    fo->Write(&it.second.w, sizeof(real_t));
    if (it.second.V) {
      if(save_aux){
        v_dim = 2*n;
      }
      fo->Write(&it.second.V, sizeof(real_t)*v_dim);
      // for(int i=0; i<v_dim; ++i){
      //   std::cout << it.second.V[i] << "\n";
      //   fo->Write(&it.second.V[i], sizeof(real_t));
      // }
    }
    if (save_aux) {
      fo->Write(&(it.second.z), sizeof(real_t));
      fo->Write(&(it.second.sqrt_g), sizeof(real_t));
    }
    saved ++ ;
  }
  LOG(INFO) << "saved " << saved << " kv pairs";
}

void SGDUpdater::Load(dmlc::Stream* fi, bool has_aux){
  std::cout << "load model \n";
  int n = param_.V_dim;
  int v_dim = n;
  int loaded=0;
  while(true){
    feaid_t id;
    SGDEntry entry;
    if(fi->Read(&id, sizeof(feaid_t))!=sizeof(feaid_t)) break;
    std::cout << id << "\n";
    fi->Read(&entry.w, sizeof(real_t));
    if(n>0){
      if(has_aux){
        v_dim = 2 * n;
      }
      entry.V = new real_t[v_dim];
      // std::memset(entry.V, 0, sizeof(real_t)*v_dim);
      std::memset(entry.V, 0, sizeof(entry.V));
      // fi->Read(&entry.V, sizeof(entry.V));
      for(int j=0; j<v_dim; ++j){
        fi->Read(&entry.V[j], sizeof(real_t));
        std::cout << entry.V[j]<< "\n";
      }
    }
    if(has_aux){
      fi->Read(&entry.z, sizeof(real_t));
      fi->Read(&entry.sqrt_g, sizeof(real_t));
    }
    model_[id] = entry;
    loaded++;
  }
  std::cout << "loaded " << loaded << "\n";
  SaveAsTxt("fm.model.txt");
}

}  // namespace difacto
