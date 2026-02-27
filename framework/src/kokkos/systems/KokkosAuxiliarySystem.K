//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAuxKernel.h"

#include "AuxiliarySystem.h"
#include "MaterialBase.h"

void
AuxiliarySystem::addKokkosKernel(const std::string & kernel_name,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  std::shared_ptr<Moose::Kokkos::AuxKernel> kernel =
      _factory.create<Moose::Kokkos::AuxKernel>(kernel_name, name, parameters, 0);

  if (kernel->isNodal())
    _kokkos_nodal_aux_storage.addObject(kernel, 0);
  else
    _kokkos_elemental_aux_storage.addObject(kernel, 0);
}

void
AuxiliarySystem::kokkosCompute(ExecFlagType type)
{
  TIME_SECTION("computeKokkosAuxKernel", 3);

  if (!_kokkos_elemental_aux_storage[type].size() && !_kokkos_nodal_aux_storage[type].size())
    return;

  auto & systems = _fe_problem.getKokkosSystems();

  // Resolve dependencies

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  auto solution_tag = _fe_problem.getVectorTagID("parallel_solution");

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    if (tag._id != solution_tag)
      needed_fe_var_vector_tags.insert(tag._id);

  associateVectorToTag(solution(), solution_tag);

  _kokkos_elemental_aux_storage[type].updateVariableDependency(needed_moose_vars);
  _kokkos_elemental_aux_storage[type].updateFEVariableCoupledVectorTagDependency(
      needed_fe_var_vector_tags);
  _kokkos_elemental_aux_storage[type].updateMatPropDependency(needed_mat_props);

  _kokkos_nodal_aux_storage[type].updateVariableDependency(needed_moose_vars);
  _kokkos_nodal_aux_storage[type].updateFEVariableCoupledVectorTagDependency(
      needed_fe_var_vector_tags);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  // Copy vectors and cache variable values at element quadature points

  systems[number()].sync(solution_tag, Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);

  for (auto & system : systems)
  {
    system.setActiveVariables(needed_moose_vars);
    system.setActiveSolutionTags(needed_fe_var_vector_tags);

    {
      TIME_SECTION("KokkosCopy", 1);
      system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
    }
    {
      TIME_SECTION("KokkosReinit", 1);
      system.reinit();
    }
  }

  systems.copyToDevice();

  {
    TIME_SECTION("KokkosMaterial", 1);

    // Compute material properties

    if (needed_mat_props.size())
    {
      _fe_problem.prepareKokkosMaterials(needed_mat_props);
      _fe_problem.reinitKokkosMaterials();
    }
  }

  {
    TIME_SECTION("KokkosAuxKernel", 1);

    // Compute auxiliary kernels

    for (auto nodal : _kokkos_nodal_aux_storage[type].getActiveObjects())
      nodal->compute();

    for (auto elemental : _kokkos_elemental_aux_storage[type].getActiveObjects())
      elemental->compute();
  }

  // Close and restore vectors

  {
    TIME_SECTION("KokkosCopy", 1);

    for (auto & system : systems)
      system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

    systems[number()].sync(solution_tag, Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

    _sys.update();
  }

  // Clear

  for (auto & system : systems)
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }

  disassociateVectorFromTag(solution(), solution_tag);
}
