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

template <typename Context>
void
dataStore(std::ostream & stream, Moose::Kokkos::MaterialPropertyStorage & storage, Context context);
template <typename Context>
void
dataLoad(std::istream & stream, Moose::Kokkos::MaterialPropertyStorage & storage, Context context);

namespace Moose::Kokkos
{

/**
 * The Kokkos class responsible for allocating and storing Kokkos material properties
 */
class MaterialPropertyStorage : protected ::MaterialPropertyStorage
{
  template <typename Context>
  friend void ::dataStore(std::ostream &, MaterialPropertyStorage &, Context);
  template <typename Context>
  friend void ::dataLoad(std::istream &, MaterialPropertyStorage &, Context);

public:
  /**
   * Constructor
   */
  MaterialPropertyStorage(MaterialPropertyRegistry & registry, FEProblemBase & problem);

  /**
   * Cast the reference of a Kokkos material property storage from the base type to the actual type
   * @param storage The Kokkos material property storage as the reference of the base type
   * @returns The Kokkos material property storage as the reference of the actual type
   */
  static MaterialPropertyStorage & cast(::MaterialPropertyStorage & storage);

  using ::MaterialPropertyStorage::addConsumer;
  using ::MaterialPropertyStorage::getConsumers;
  using ::MaterialPropertyStorage::getMaterialData;
  using ::MaterialPropertyStorage::getMaterialPropertyRegistry;
  using ::MaterialPropertyStorage::hasStatefulProperties;
  using ::MaterialPropertyStorage::setRecovering;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Add a material property
   * @param prop_name The property name
   * @param type The property data type
   * @param state The property state
   * @param declarer The Kokkos material declaring the property, nullptr if simply reserving the
   * property
   * @param shell The managed pointer containing the instance of the property
   * @returns The material property
   */
  MaterialPropertyBase & addKokkosProperty(const std::string & prop_name,
                                           const std::type_info & type,
                                           const unsigned int state,
                                           const ::MaterialBase * declarer,
                                           std::shared_ptr<MaterialPropertyBase> shell);
  /**
   * Add an old/older material property
   * @param prop_name The property name
   * @param state The property state
   * @param shell The managed pointer containing the
   * @returns The material property
   */
  MaterialPropertyBase & addKokkosPropertyState(const std::string & prop_name,
                                                const unsigned int state,
                                                std::shared_ptr<MaterialPropertyBase> shell);
  /**
   * Declare a material property
   * @param prop_name The property name
   * @param type The property data type
   * @param declarer The Kokkos material declaring the property, nullptr if simply reserving the
   * @param dims The vector containing the size of each dimension
   * @param bnd Whether the property is a face property
   * @param on_demand Whether the property is an on-demand property
   * @param constant_option Whether the property is constant on element or subdomain
   * @param shell The managed pointer containing the instance of the property
   * @returns The material property
   */
  MaterialPropertyBase & declareKokkosProperty(const std::string & prop_name,
                                               const std::type_info & type,
                                               const ::MaterialBase * declarer,
                                               const std::vector<unsigned int> & dims,
                                               const bool bnd,
                                               const bool on_demand,
                                               const PropertyConstantOption constant_option,
                                               std::shared_ptr<MaterialPropertyBase> shell);
  /**
   * Get a material property
   * @param prop_name The property name
   * @param state The property state
   * @returns The material property
   */
  MaterialPropertyBase & getKokkosProperty(std::string prop_name, unsigned int state = 0);
  /**
   * Get whether a material property exists
   * @param prop_name The property name
   * @param state The property state
   * @returns Whether the material property exists
   */
  bool haveKokkosProperty(std::string prop_name, unsigned int state = 0);
  /**
   * Register the load/store functions
   * @param type The property type index
   * @param store The function pointer to the store function
   * @param load The function pointer to the load function
   */
  void registerLoadStore(std::type_index type, PropertyStore store, PropertyLoad load);
  /**
   * Allocate all the material property data storages
   */
  void allocateKokkosProperties();
#endif

  /**
   * Shift current, old, and older material property data storages
   */
  void shift();
  /**
   * Copy current material properties to old and older
   */
  void copy();

private:
  /**
   * Material properties
   */
  std::unordered_map<std::string, std::shared_ptr<MaterialPropertyBase>>
      _kokkos_props[MaterialData::max_state + 1];
  /**
   * Record of each material property
   */
  std::unordered_map<std::string, Moose::Kokkos::PropRecord> _kokkos_prop_records;
  /**
   * Function pointer maps for load/store
   */
  ///@{
  static std::unordered_map<std::type_index, PropertyStore> _store_functions;
  static std::unordered_map<std::type_index, PropertyLoad> _load_functions;
  ///@}
};

} // namespace Moose::Kokkos
