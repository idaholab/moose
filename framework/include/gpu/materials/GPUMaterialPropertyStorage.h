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

void
dataStore(std::ostream & stream, Moose::Kokkos::MaterialPropertyStorage & storage, void * context);
void
dataLoad(std::istream & stream, Moose::Kokkos::MaterialPropertyStorage & storage, void * context);

namespace Moose
{
namespace Kokkos
{

class MaterialPropertyStorage : protected ::MaterialPropertyStorage
{
  friend void ::dataStore(std::ostream &, MaterialPropertyStorage &, void *);
  friend void ::dataLoad(std::istream &, MaterialPropertyStorage &, void *);

public:
  MaterialPropertyStorage(MaterialPropertyRegistry & registry, FEProblemBase & problem)
    : ::MaterialPropertyStorage(registry, problem)
  {
  }

  static MaterialPropertyStorage & cast(::MaterialPropertyStorage & storage)
  {
    return static_cast<MaterialPropertyStorage &>(storage);
  }

  using ::MaterialPropertyStorage::addConsumer;
  using ::MaterialPropertyStorage::getConsumers;
  using ::MaterialPropertyStorage::getMaterialData;
  using ::MaterialPropertyStorage::getMaterialPropertyRegistry;
  using ::MaterialPropertyStorage::hasStatefulProperties;

#ifdef MOOSE_KOKKOS_SCOPE
  // Add a Kokkos material property to the storage
  MaterialPropertyBase & addKokkosProperty(const std::string & prop_name,
                                           const std::type_info & type,
                                           const unsigned int state,
                                           const MaterialBase * declarer,
                                           std::shared_ptr<MaterialPropertyBase> shell);
  MaterialPropertyBase & addKokkosPropertyState(const std::string & prop_name,
                                                const unsigned int state,
                                                std::shared_ptr<MaterialPropertyBase> shell);
  MaterialPropertyBase & declareKokkosProperty(const std::string & prop_name,
                                               const std::type_info & type,
                                               const unsigned int state,
                                               const MaterialBase * declarer,
                                               const std::vector<unsigned int> & dims,
                                               const bool bnd,
                                               std::shared_ptr<MaterialPropertyBase> shell);
  // Get a Kokkos material property
  MaterialPropertyBase & getKokkosProperty(std::string prop_name, unsigned int state = 0);
  // Allocate all the Kokkos material property storages
  void allocateKokkosProperties();
  // Check whether a Kokkos material property exists
  bool haveKokkosProperty(std::string prop_name, unsigned int state = 0);
#endif

  void shift();
  void copy();

private:
  // Kokkos material properties
  std::map<std::string, std::shared_ptr<MaterialPropertyBase>>
      _kokkos_props[MaterialData::max_state + 1];
  // Records of each Kokkos material property
  std::map<std::string, Moose::Kokkos::PropRecord> _kokkos_prop_records;
};

} // namespace Kokkos
} // namespace Moose
