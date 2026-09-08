//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSystem.h"

#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "AuxiliarySystem.h"

#include "libmesh/system.h"

namespace Moose::Kokkos
{
System::System(SystemBase & system)
  : PerfGraphInterface(system.feProblem().getMooseApp().perfGraph(), "KokkosSystem"),
    DofSpace(system.mesh(), system.system()),
    _system(system)
{
  setupVariables();
  setupTags();

  if (!dynamic_cast<AuxiliarySystem *>(&_system))
    setupSparsity();
}

void
System::setupVariables()
{
  auto & sys = _system.system();

  _var_subdomain_active.create(_num_vars, kokkosMesh().getNumSubdomains());

  for (unsigned int var = 0; var < _num_vars; ++var)
    for (auto subdomain : _mesh.meshSubdomains())
      _var_subdomain_active(var, kokkosMesh().getContiguousSubdomainID(subdomain)) =
          sys.variable(var).active_on_subdomain(subdomain);

  _var_subdomain_active.copyToDevice();
}

void
System::setupTags()
{
  _residual_tag_active.create(MAX_TAG);
  _residual_tag_active = false;

  _matrix_tag_active.create(MAX_TAG);
  _matrix_tag_active = false;

  _vectors.create(MAX_TAG);
  _matrices.create(MAX_TAG);
}

void
System::sync(const MemcpyType dir)
{
  if (dir == MemcpyType::HOST_TO_DEVICE)
  {
    for (auto tag : _active_solution_tags)
    {
      auto & vector = _system.getVector(tag);

      _vectors[tag].create(vector, *this, false);
      _vectors[tag].copyToDevice();
    }

    for (auto tag : _active_residual_tags)
    {
      auto & vector = _system.getVector(tag);

      _vectors[tag].create(vector, *this, true);
      _vectors[tag].copyToDevice();
    }

    for (auto tag : _active_matrix_tags)
    {
      auto & matrix = _system.getMatrix(tag);

      {
        TIME_SECTION("MatSetPreallocation", 1);
        _matrices[tag].create(matrix, *this);
      }

      _matrices[tag] = 0;
    }

    _vectors.copyToDevice();
    _matrices.copyToDevice();
  }
  else if (dir == MemcpyType::DEVICE_TO_HOST)
  {
    for (auto tag : _active_solution_tags)
      _vectors[tag].restore();

    for (auto tag : _active_residual_tags)
      _vectors[tag].close();

    for (auto tag : _active_matrix_tags)
    {
      TIME_SECTION("MatSetValues", 1);
      _matrices[tag].close();
    }
  }
}

void
System::sync(const std::set<TagID> & tags, const MemcpyType dir)
{
  if (dir == MemcpyType::HOST_TO_DEVICE)
  {
    for (auto tag : tags)
    {
      if (!_system.hasVector(tag))
        continue;

      auto & vector = _system.getVector(tag);

      _vectors[tag].create(vector, *this, false);
      _vectors[tag].copyToDevice();
    }

    _vectors.copyToDevice();
  }
  else if (dir == MemcpyType::DEVICE_TO_HOST)
  {
    for (auto tag : tags)
    {
      if (!_system.hasVector(tag))
        continue;

      _vectors[tag].copyToHost();
      _vectors[tag].restore();
    }
  }
}

void
System::sync(const std::vector<TagID> & tags, const MemcpyType dir)
{
  sync(std::set<TagID>(tags.begin(), tags.end()), dir);
}

void
System::sync(const TagID tag, const MemcpyType dir)
{
  sync(std::set<TagID>{tag}, dir);
}

void
System::setActiveVariables(const std::set<MooseVariableFieldBase *> & vars)
{
  std::set<unsigned int> active_variables;

  for (auto var : vars)
    if (var->sys().number() == _system.number())
      for (unsigned int i = 0; i < var->count(); ++i)
        active_variables.insert(var->number() + i);

  _active_variables = active_variables;
}

void
System::setActiveSolutionTags(const std::set<TagID> & tags)
{
  std::set<TagID> active_solution_tags;

  for (auto tag : tags)
    if (_system.hasVector(tag))
      active_solution_tags.insert(tag);

  _active_solution_tags = active_solution_tags;
}

void
System::setActiveResidualTags(const std::set<TagID> & tags)
{
  std::set<TagID> active_residual_tags;

  for (auto tag : tags)
    if (_system.hasVector(tag))
    {
      active_residual_tags.insert(tag);
      _residual_tag_active[tag] = true;
    }

  _active_residual_tags = active_residual_tags;

  _residual_tag_active.copyToDevice();
}

void
System::setActiveMatrixTags(const std::set<TagID> & tags)
{
  std::set<TagID> active_matrix_tags;

  for (auto tag : tags)
    if (_system.hasMatrix(tag))
    {
      active_matrix_tags.insert(tag);
      _matrix_tag_active[tag] = true;
    }

  _active_matrix_tags = active_matrix_tags;

  _matrix_tag_active.copyToDevice();
}

} // namespace Moose::Kokkos
