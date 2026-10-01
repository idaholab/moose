//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  // Kokkos aux kernels do not use execution_order_group (suppressed at 0 in validParams()), but
  // compute() only calls kokkosCompute() while iterating _execution_order_groups, so group 0 must
  // be present or Kokkos aux kernels would never execute.
  _execution_order_groups.insert(0);
}

AuxiliarySystem::KokkosAuxStage::KokkosAuxStage()
  : nodal(/*num_threads=*/1), elemental(/*num_threads=*/1)
{
}

void
AuxiliarySystem::buildKokkosAuxStages()
{
  _kokkos_aux_stages.clear();

  for (const auto & [type, nodal_storage] : _kokkos_nodal_aux_storage)
  {
    const auto & elemental_storage = std::as_const(_kokkos_elemental_aux_storage)[type];
    std::vector<KokkosAuxStage> stages;
    // Latest stage writing each variable; a variable written by several block-restricted kernels is
    // ready only after the last of them
    std::map<std::string, unsigned int> var_stage;

    // Nodal kernels cannot couple elemental variables, so computing the nodal kernels of a stage
    // before its elemental kernels respects all dependencies between the two kinds
    for (const bool nodal : {true, false})
      for (const auto & kernel : (nodal ? nodal_storage : elemental_storage).getActiveObjects())
      {
        // A kernel coupling the current value of a variable must run in a later stage than the
        // kernels writing it, because the written values only reach the coupled solution vector
        // (including ghosts) and the cached quadrature point values once a stage is copied back
        // to the host and the system is updated
        unsigned int stage = 0;
        for (const auto & var : kernel->getRequestedItems())
          if (const auto it = var_stage.find(var); it != var_stage.end())
            stage = std::max(stage, it->second + 1);

        if (stage >= stages.size())
          stages.resize(stage + 1);
        (nodal ? stages[stage].nodal : stages[stage].elemental)
            .addObject(kernel, /*tid=*/0, /*recurse=*/false);

        for (const auto & var : kernel->getSuppliedItems())
          var_stage[var] = std::max(var_stage[var], stage);
      }

    if (stages.size())
      _kokkos_aux_stages.emplace(type, std::move(stages));
  }
}

void
AuxiliarySystem::kokkosCompute(ExecFlagType type, int /*group*/)
{
  TIME_SECTION("computeKokkosAuxKernel", 3);

  const auto stages = _kokkos_aux_stages.find(type);
  if (stages == _kokkos_aux_stages.end())
    return;

  auto & systems = _fe_problem.getKokkosFESystems();

  auto solution_tag = _fe_problem.getVectorTagID("parallel_solution");

  associateVectorToTag(solution(), solution_tag);

  for (const auto & stage : stages->second)
  {
    // Resolve dependencies

    std::set<MooseVariableFieldBase *> needed_moose_vars;
    std::set<TagID> needed_fe_var_vector_tags;
    std::unordered_set<unsigned int> needed_mat_props;

    for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
      if (tag._id != solution_tag)
        needed_fe_var_vector_tags.insert(tag._id);

    // Nodal kernels read coupled values directly from DOFs and cannot use material properties, so
    // only elemental kernels need variable values cached at quadrature points
    stage.elemental.updateVariableDependency(needed_moose_vars);
    stage.elemental.updateMatPropDependency(needed_mat_props);
    stage.elemental.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
    stage.nodal.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

    if (needed_mat_props.size())
    {
      _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
      _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
          needed_fe_var_vector_tags);
    }

    // Copy vectors and cache variable values at element quadature points

    systems[number()].sync(solution_tag, Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);

    for (auto & system : systems.constructedEntries())
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

      for (const auto * const kernels : {&stage.nodal, &stage.elemental})
        for (const auto & kernel : kernels->getActiveObjects())
          kernel->compute();
    }

    // Close and restore vectors

    {
      TIME_SECTION("KokkosCopy", 1);

      for (auto & system : systems.constructedEntries())
        system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

      systems[number()].sync(solution_tag, Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

      _sys.update();
    }
  }

  // Clear

  for (auto & system : systems.constructedEntries())
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }

  disassociateVectorFromTag(solution(), solution_tag);
}
