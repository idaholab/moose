//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialPropertyStorage.h"

class FEProblemBase;
class GPUMaterialPropertyStorage;

void dataStore(std::ostream & stream, GPUMaterialPropertyStorage & storage, void * context);
void dataLoad(std::istream & stream, GPUMaterialPropertyStorage & storage, void * context);

class GPUMaterialPropertyStorage : protected MaterialPropertyStorage
{
  friend void dataStore(std::ostream &, GPUMaterialPropertyStorage &, void *);
  friend void dataLoad(std::istream &, GPUMaterialPropertyStorage &, void *);

public:
  GPUMaterialPropertyStorage(MaterialPropertyRegistry & registry, FEProblemBase & problem)
    : MaterialPropertyStorage(registry, problem)
  {
  }

  static GPUMaterialPropertyStorage & cast(MaterialPropertyStorage & storage)
  {
    return static_cast<GPUMaterialPropertyStorage &>(storage);
  }

  using MaterialPropertyStorage::addConsumer;
  using MaterialPropertyStorage::getConsumers;
  using MaterialPropertyStorage::getMaterialData;
  using MaterialPropertyStorage::getMaterialPropertyRegistry;
  using MaterialPropertyStorage::hasStatefulProperties;

#ifdef MOOSE_GPU_SCOPE
  // Add a GPU material property to the storage
  GPUMaterialPropertyBase & addGPUProperty(const std::string & prop_name,
                                           const std::type_info & type,
                                           const unsigned int state,
                                           const MaterialBase * declarer,
                                           std::shared_ptr<GPUMaterialPropertyBase> shell);
  GPUMaterialPropertyBase & addGPUPropertyState(const std::string & prop_name,
                                                const unsigned int state,
                                                std::shared_ptr<GPUMaterialPropertyBase> shell);
  GPUMaterialPropertyBase & declareGPUProperty(const std::string & prop_name,
                                               const std::type_info & type,
                                               const unsigned int state,
                                               const MaterialBase * declarer,
                                               const std::vector<unsigned int> & dims,
                                               const bool bnd,
                                               std::shared_ptr<GPUMaterialPropertyBase> shell);
  // Get a GPU material property
  GPUMaterialPropertyBase & getGPUProperty(std::string prop_name, unsigned int state = 0);
  // Allocate all the GPU material property storages
  void allocateGPUProperties();
  // Check whether a GPU material property exists
  bool haveGPUProperty(std::string prop_name, unsigned int state = 0);
#endif

  void shift();
  void copy();

private:
  // GPU material properties
  std::map<std::string, std::shared_ptr<GPUMaterialPropertyBase>>
      _gpu_props[MaterialData::max_state + 1];
  // Records of each GPU material property
  std::map<std::string, GPUPropRecord> _gpu_prop_records;
};
