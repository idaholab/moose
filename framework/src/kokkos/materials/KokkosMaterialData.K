//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMaterialPropertyStorage.h"

#include "MaterialData.h"

Moose::Kokkos::MaterialPropertyBase &
MaterialData::addKokkosPropertyHelper(const std::string & prop_name,
                                      const std::type_info & type,
                                      const unsigned int state,
                                      std::shared_ptr<Moose::Kokkos::MaterialPropertyBase> shell)
{
  auto & storage = Moose::Kokkos::MaterialPropertyStorage::cast(_storage);

  return storage.addKokkosProperty(prop_name, type, state, nullptr, shell);
}

Moose::Kokkos::MaterialPropertyBase &
MaterialData::declareKokkosPropertyHelper(
    const std::string & prop_name,
    const std::type_info & type,
    const MaterialBase * declarer,
    const std::vector<unsigned int> & dims,
    const bool bnd,
    const bool on_demand,
    const Moose::Kokkos::PropertyConstantOption constant_option,
    std::shared_ptr<Moose::Kokkos::MaterialPropertyBase> shell)
{
  auto & storage = Moose::Kokkos::MaterialPropertyStorage::cast(_storage);

  return storage.declareKokkosProperty(
      prop_name, type, declarer, dims, bnd, on_demand, constant_option, shell);
}

Moose::Kokkos::MaterialPropertyBase &
MaterialData::getKokkosPropertyHelper(
    const std::string & prop_name,
    const unsigned int state,
    std::shared_ptr<Moose::Kokkos::MaterialPropertyBase> shell) const
{
  auto & storage = Moose::Kokkos::MaterialPropertyStorage::cast(_storage);

  if (state)
    return storage.addKokkosPropertyState(prop_name, state, shell);
  else
    return storage.getKokkosProperty(prop_name);
}

bool
MaterialData::haveKokkosPropertyHelper(const std::string & prop_name) const
{
  auto & storage = Moose::Kokkos::MaterialPropertyStorage::cast(_storage);

  return storage.haveKokkosProperty(prop_name);
}

void
MaterialData::kokkosRegisterLoadStoreHelper(std::type_index type,
                                            Moose::Kokkos::PropertyStore store,
                                            Moose::Kokkos::PropertyLoad load)
{
  auto & storage = Moose::Kokkos::MaterialPropertyStorage::cast(_storage);

  return storage.registerLoadStore(type, store, load);
}
