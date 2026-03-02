//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMaterialPropertyStorage.h"

#include "FEProblemBase.h"
#include "MaterialBase.h"

namespace Moose::Kokkos
{

std::unordered_map<std::type_index, PropertyStore> MaterialPropertyStorage::_store_functions;
std::unordered_map<std::type_index, PropertyLoad> MaterialPropertyStorage::_load_functions;

MaterialPropertyStorage::MaterialPropertyStorage(MaterialPropertyRegistry & registry,
                                                 FEProblemBase & problem)
  : ::MaterialPropertyStorage(registry, problem)
{
}

MaterialPropertyStorage &
MaterialPropertyStorage::cast(::MaterialPropertyStorage & storage)
{
  return static_cast<MaterialPropertyStorage &>(storage);
}

MaterialPropertyBase &
MaterialPropertyStorage::addKokkosProperty(const std::string & prop_name,
                                           const std::type_info & type,
                                           const unsigned int state,
                                           const ::MaterialBase * declarer,
                                           std::shared_ptr<MaterialPropertyBase> shell)
{
  auto id = addProperty(prop_name, type, state, declarer);

  if (state)
    return addKokkosPropertyState(prop_name, state, shell);

  auto & record = _kokkos_prop_records[prop_name];

  if (record.id == libMesh::invalid_uint)
  {
    record.type = MooseUtils::prettyCppType(libMesh::demangle(type.name()));
    record.id = id;
  }

  if (declarer)
    record.declarers.insert(declarer);

  if (!_kokkos_props[0].count(prop_name))
  {
    _kokkos_props[0][prop_name] = shell;
    _kokkos_props[0][prop_name]->init(record, {});
  }

  return *_kokkos_props[0][prop_name];
}

MaterialPropertyBase &
MaterialPropertyStorage::addKokkosPropertyState(const std::string & prop_name,
                                                const unsigned int state,
                                                std::shared_ptr<MaterialPropertyBase> shell)
{
  auto & record = _kokkos_prop_records[prop_name];

  if (!_kokkos_props[state].count(prop_name))
  {
    _kokkos_props[state][prop_name] = shell;
    _kokkos_props[state][prop_name]->init(record, {});
  }

  return *_kokkos_props[state][prop_name];
}

MaterialPropertyBase &
MaterialPropertyStorage::declareKokkosProperty(const std::string & prop_name,
                                               const std::type_info & type,
                                               const ::MaterialBase * declarer,
                                               const std::vector<unsigned int> & dims,
                                               const bool bnd,
                                               const bool on_demand,
                                               const PropertyConstantOption constant_option,
                                               std::shared_ptr<MaterialPropertyBase> shell)
{
  auto & record = _kokkos_prop_records[prop_name];

  if (record.declarers.size())
  {
    if (record.dims != dims)
    {
      std::stringstream ss;

      ss << "The declared " << dims.size() << "D Kokkos material property '" << prop_name << "'";
      if (dims.size())
        ss << " with dimensions (" << Moose::stringify(dims) << ")";
      ss << " was already declared as a " << record.dims.size() << "D property";
      if (record.dims.size())
        ss << " with dimensions (" << Moose::stringify(record.dims) << ")";
      ss << ".";

      mooseError(ss.str());
    }

    if (record.on_demand != on_demand)
      mooseError(
          "The Kokkos material property '",
          prop_name,
          "' is being declared by multiple materials, and some of them are declaring it as an "
          "on-demand property while others are not.");

    if (record.constant_option != constant_option)
      mooseError(
          "The Kokkos material property '",
          prop_name,
          "' is being declared by multiple materials with different 'constant_on' parameters.");
  }
  else
  {
    record.dims = dims;
    record.bnd = bnd;
    record.on_demand = on_demand;
    record.constant_option = constant_option;
  }

  return addKokkosProperty(prop_name, type, 0, declarer, shell);
}

MaterialPropertyBase &
MaterialPropertyStorage::getKokkosProperty(std::string prop_name, unsigned int state)
{
  return *_kokkos_props[state][prop_name];
}

void
MaterialPropertyStorage::allocateKokkosProperties()
{
  auto & mesh = _problem.mesh();
  auto & assembly = _problem.kokkosAssembly();

  for (unsigned int state = 0; state < numStates(); ++state)
    for (auto & [name, prop] : _kokkos_props[state])
    {
      auto & record = _kokkos_prop_records[name];

      if (!record.on_demand || _problem.isMatPropRequested(name))
        for (auto * const declarer : record.declarers)
        {
          auto & blocks =
              declarer->boundaryRestricted() ? declarer->meshBlockIDs() : declarer->blockIDs();

          prop->allocate(mesh, assembly, blocks, record.bnd, {});
        }
    }
}

bool
MaterialPropertyStorage::haveKokkosProperty(std::string prop_name, unsigned int state)
{
  return _kokkos_props[state].count(prop_name);
}

void
MaterialPropertyStorage::registerLoadStore(std::type_index type,
                                           PropertyStore store,
                                           PropertyLoad load)
{
  _store_functions[type] = store;
  _load_functions[type] = load;
}

void
MaterialPropertyStorage::shift()
{
  for (unsigned int state = maxState(); state != 0; --state)
    for (auto & [name, prop] : _kokkos_props[state])
      prop->swap(*_kokkos_props[state - 1][name], {});
}

void
MaterialPropertyStorage::copy()
{
  for (unsigned int state : statefulIndexRange())
    for (auto & [name, prop] : _kokkos_props[state])
      if (!isRestoredProperty(name))
        prop->copy(*_kokkos_props[0][name], {});
}

} // namespace Moose::Kokkos

void
dataStore(std::ostream & stream, Moose::Kokkos::MaterialPropertyStorage & storage, void * context)
{
  auto & base = static_cast<MaterialPropertyStorage &>(storage);

  dataStore(stream, base, context);

  auto & props = storage._kokkos_props;
  auto & records = storage._kokkos_prop_records;

  unsigned int num_stateful_properties = 0;

  for (auto & prop : props[0])
  {
    auto name = prop.first;
    auto id = records[name].id;

    if (storage.getPropRecord(id).stateful())
      num_stateful_properties++;
  }

  dataStore(stream, num_stateful_properties, nullptr);

  for (auto & prop : props[0])
  {
    auto name = prop.first;
    auto id = records[name].id;

    if (storage.getPropRecord(id).stateful())
    {
      dataStore(stream, name, nullptr);

      unsigned int num_states = storage.getPropRecord(id).state + 1;
      dataStore(stream, num_states, nullptr);

      // Store data in a temporary stream to allow skipping loading
      std::stringstream ss;

      for (unsigned int state = 0; state < num_states; ++state)
      {
        auto it = storage._store_functions.find(props[state][name]->propertyType());
        mooseAssert(it != storage._store_functions.end(),
                    "Store function was not properly registered.");

        it->second(ss, props[state][name].get());
      }

      // Write the temporary stream to the output stream
      dataStore(stream, ss, nullptr);
    }
  }
}

void
dataLoad(std::istream & stream, Moose::Kokkos::MaterialPropertyStorage & storage, void * context)
{
  auto & base = static_cast<MaterialPropertyStorage &>(storage);

  // All exceptions are expected to be handled here
  dataLoad(stream, base, context);

  auto & props = storage._kokkos_props;
  auto & records = storage._kokkos_prop_records;

  unsigned int num_stateful_properties;
  dataLoad(stream, num_stateful_properties, nullptr);

  for (unsigned int prop = 0; prop < num_stateful_properties; ++prop)
  {
    std::string name;
    dataLoad(stream, name, nullptr);

    unsigned int num_states;
    dataLoad(stream, num_states, nullptr);

    std::stringstream ss;
    dataLoad(stream, ss, nullptr);

    if (records.count(name) && storage.getPropRecord(records[name].id).stateful())
      for (unsigned int state = 0; state < num_states; ++state)
      {
        auto it = storage._load_functions.find(props[state][name]->propertyType());
        mooseAssert(it != storage._load_functions.end(),
                    "Load function was not properly registered.");

        it->second(ss, props[state][name].get());
      }
  }
}
