//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMaterialPropertyStorage.h"

#include "FEProblemBase.h"
#include "MaterialBase.h"

GPUMaterialPropertyBase &
GPUMaterialPropertyStorage::addGPUProperty(const std::string & prop_name,
                                           const std::type_info & type,
                                           const unsigned int state,
                                           const MaterialBase * declarer,
                                           std::shared_ptr<GPUMaterialPropertyBase> shell)
{
  auto id = addProperty(prop_name, type, state, declarer);

  if (state)
    return addGPUPropertyState(prop_name, state, shell);

  auto & record = _gpu_prop_records[prop_name];

  if (record.id == libMesh::invalid_uint)
  {
    record.type = MooseUtils::prettyCppType(libMesh::demangle(type.name()));
    record.id = id;
  }

  if (declarer)
    record.declarers.insert(declarer);

  if (!_gpu_props[0].count(prop_name))
  {
    _gpu_props[0][prop_name] = shell;
    _gpu_props[0][prop_name]->init(record);
  }

  return *_gpu_props[0][prop_name];
}

GPUMaterialPropertyBase &
GPUMaterialPropertyStorage::addGPUPropertyState(const std::string & prop_name,
                                                const unsigned int state,
                                                std::shared_ptr<GPUMaterialPropertyBase> shell)
{
  auto & record = _gpu_prop_records[prop_name];

  if (!_gpu_props[state].count(prop_name))
  {
    _gpu_props[state][prop_name] = shell;
    _gpu_props[state][prop_name]->init(record);
  }

  return *_gpu_props[state][prop_name];
}

GPUMaterialPropertyBase &
GPUMaterialPropertyStorage::declareGPUProperty(const std::string & prop_name,
                                               const std::type_info & type,
                                               const unsigned int state,
                                               const MaterialBase * declarer,
                                               const std::vector<unsigned int> & dims,
                                               const bool bnd,
                                               std::shared_ptr<GPUMaterialPropertyBase> shell)
{
  auto & record = _gpu_prop_records[prop_name];

  if (record.declarers.size() && record.dims != dims)
  {
    std::stringstream ss;

    ss << "The declared " << dims.size() << "D GPU material property '" << prop_name << "'";
    if (dims.size())
      ss << " with dimensions (" << Moose::stringify(dims) << ")";
    ss << " was already declared as a " << record.dims.size() << "D property";
    if (record.dims.size())
      ss << " with dimensions (" << Moose::stringify(record.dims) << ")";
    ss << ".";

    mooseError(ss.str());
  }

  if (!record.declarers.size())
  {
    record.dims = dims;
    record.bnd = bnd;
  }

  return addGPUProperty(prop_name, type, state, declarer, shell);
}

GPUMaterialPropertyBase &
GPUMaterialPropertyStorage::getGPUProperty(std::string prop_name, unsigned int state)
{
  return *_gpu_props[state][prop_name];
}

void
GPUMaterialPropertyStorage::allocateGPUProperties()
{
  auto & mesh = _problem.mesh();
  auto & assembly = _problem.gpuAssembly();

  for (unsigned int state = 0; state < numStates(); ++state)
    for (auto & [name, prop] : _gpu_props[state])
    {
      auto & record = _gpu_prop_records[name];

      for (auto declarer : record.declarers)
      {
        auto & blocks =
            declarer->boundaryRestricted() ? declarer->meshBlockIDs() : declarer->blockIDs();

        prop->allocate(mesh, assembly, blocks, record.bnd);
      }
    }
}

bool
GPUMaterialPropertyStorage::haveGPUProperty(std::string prop_name, unsigned int state)
{
  return _gpu_props[state].count(prop_name);
}

void
GPUMaterialPropertyStorage::shift()
{
  for (unsigned int state = maxState(); state != 0; --state)
    for (auto & [name, prop] : _gpu_props[state])
      prop->swap(*_gpu_props[state - 1][name]);
}

void
GPUMaterialPropertyStorage::copy()
{
  for (unsigned int state : statefulIndexRange())
    for (auto & [name, prop] : _gpu_props[state])
      prop->copy(*_gpu_props[0][name]);
}

void
dataStore(std::ostream & stream, GPUMaterialPropertyStorage & storage, void * context)
{
  auto & base = static_cast<MaterialPropertyStorage &>(storage);

  dataStore(stream, base, context);

  auto & props = storage._gpu_props;
  auto & records = storage._gpu_prop_records;

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
        props[state][name]->store(ss);

      // Write the temporary stream to the output stream
      dataStore(stream, ss, nullptr);
    }
  }
}

void
dataLoad(std::istream & stream, GPUMaterialPropertyStorage & storage, void * context)
{
  auto & base = static_cast<MaterialPropertyStorage &>(storage);

  // All exceptions are expected to be handled here
  dataLoad(stream, base, context);

  auto & props = storage._gpu_props;
  auto & records = storage._gpu_prop_records;

  unsigned int num_stateful_properties;
  dataLoad(stream, num_stateful_properties, nullptr);

  for (auto prop = 0; prop < num_stateful_properties; ++prop)
  {
    std::string name;
    dataLoad(stream, name, nullptr);

    unsigned int num_states;
    dataLoad(stream, num_states, nullptr);

    std::stringstream ss;
    dataLoad(stream, ss, nullptr);

    if (records.count(name) && storage.getPropRecord(records[name].id).stateful())
      for (unsigned int state = 0; state < num_states; ++state)
        props[state][name]->load(ss);
  }
}
