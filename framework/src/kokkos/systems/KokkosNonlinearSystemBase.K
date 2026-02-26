//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosKernelBase.h"
#include "KokkosNodalKernelBase.h"
#include "KokkosNodalBCBase.h"
#include "KokkosIntegratedBCBase.h"

#include "MaterialBase.h"
#include "NonlinearSystemBase.h"
#include "MooseVariableFieldBase.h"
#include "FEProblemBase.h"

void
NonlinearSystemBase::addKokkosKernel(const std::string & kernel_name,
                                     const std::string & name,
                                     InputParameters & parameters)
{
  // Create the kernel object via the factory and add to warehouse
  std::shared_ptr<Moose::Kokkos::KernelBase> kernel =
      _factory.create<Moose::Kokkos::KernelBase>(kernel_name, name, parameters);
  _kokkos_kernels.addObject(kernel, 0);
  postAddResidualObject(*kernel);
}

void
NonlinearSystemBase::addKokkosNodalKernel(const std::string & kernel_name,
                                          const std::string & name,
                                          InputParameters & parameters)
{
  // Create the kernel object via the factory and add to warehouse
  std::shared_ptr<Moose::Kokkos::NodalKernelBase> kernel =
      _factory.create<Moose::Kokkos::NodalKernelBase>(kernel_name, name, parameters);
  _kokkos_nodal_kernels.addObject(kernel, 0);
  postAddResidualObject(*kernel);
}

void
NonlinearSystemBase::addKokkosBoundaryCondition(const std::string & bc_name,
                                                const std::string & name,
                                                InputParameters & parameters)
{
  // Create the object
  std::shared_ptr<Moose::Kokkos::BoundaryCondition> bc =
      _factory.create<Moose::Kokkos::BoundaryCondition>(bc_name, name, parameters, 0);
  postAddResidualObject(*bc);

  // Active BoundaryIDs for the object
  const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  auto bc_var = dynamic_cast<const MooseVariableFieldBase *>(&bc->variable());
  _vars[0].addBoundaryVar(boundary_ids, bc_var);

  // Cast to the various types of BCs
  std::shared_ptr<Moose::Kokkos::NodalBCBase> nbc =
      std::dynamic_pointer_cast<Moose::Kokkos::NodalBCBase>(bc);
  std::shared_ptr<Moose::Kokkos::IntegratedBCBase> ibc =
      std::dynamic_pointer_cast<Moose::Kokkos::IntegratedBCBase>(bc);

  // NodalBCBase
  if (nbc)
  {
    if (!nbc->variable().isNodal())
      mooseError("Trying to use nodal boundary condition '",
                 nbc->name(),
                 "' on a non-nodal variable '",
                 nbc->variable().name(),
                 "'.");

    _kokkos_nodal_bcs.addObject(nbc);
    _vars[0].addBoundaryVars(boundary_ids, nbc->getCoupledVars());

    // DirichletBCs that are preset
    if (nbc->preset())
      _kokkos_preset_nodal_bcs.addObject(nbc);
  }

  // IntegratedBCBase
  else if (ibc)
  {
    _kokkos_integrated_bcs.addObject(ibc);
    _vars[0].addBoundaryVars(boundary_ids, ibc->getCoupledVars());
  }

  else
    mooseError("Unknown Kokkos BoundaryCondition type for object named ", bc->name());
}

void
NonlinearSystemBase::setKokkosInitialSolution()
{
  // The preset Dirichlet BCs can have coupled variables from other systems, so we grab all systems
  auto & systems = _fe_problem.getKokkosSystems();

  auto tag = _fe_problem.addVectorTag("parallel_solution", Moose::VECTOR_TAG_SOLUTION);

  associateVectorToTag(solution(), tag);

  std::set<TagID> needed_fe_var_vector_tags = {tag};

  for (auto boundary : _fe_problem.mesh().meshBoundaryIds())
    _kokkos_preset_nodal_bcs.updateBoundaryFEVariableCoupledVectorTagDependency(
        boundary, needed_fe_var_vector_tags);

  for (auto & system : systems)
    system.sync(needed_fe_var_vector_tags, Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);

  systems.copyToDevice();

  for (auto nbc : _kokkos_preset_nodal_bcs.getActiveObjects())
    std::static_pointer_cast<Moose::Kokkos::NodalBCBase>(nbc)->presetSolution(tag);

  Kokkos::fence();

  for (auto & system : systems)
    system.sync(needed_fe_var_vector_tags, Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

  disassociateVectorFromTag(solution(), tag);
}

void
NonlinearSystemBase::computeKokkosResidual(const std::set<TagID> & tags)
{
  TIME_SECTION("computeKokkosResidual", 1);

  auto & systems = _fe_problem.getKokkosSystems();

  systems[number()].setActiveResidualTags(tags);

  // Resolve dependencies

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  _kokkos_kernels.updateVariableDependency(needed_moose_vars);
  _kokkos_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  _kokkos_kernels.updateMatPropDependency(needed_mat_props);

  _kokkos_integrated_bcs.updateVariableDependency(needed_moose_vars);
  _kokkos_integrated_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  _kokkos_integrated_bcs.updateMatPropDependency(needed_mat_props);

  _kokkos_nodal_bcs.updateVariableDependency(needed_moose_vars);
  _kokkos_nodal_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  // Copy data and cache variable values at element quadature points

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
    TIME_SECTION("KokkosKernel", 1);

    // Compute kernels

    for (auto kernel : _kokkos_kernels.getVectorTagsObjectWarehouse(tags, 0).getActiveObjects())
      kernel->computeResidual();

    for (auto nodal_kernel :
         _kokkos_nodal_kernels.getVectorTagsObjectWarehouse(tags, 0).getActiveObjects())
      nodal_kernel->computeResidual();

    for (auto ibc : _kokkos_integrated_bcs.getVectorTagsObjectWarehouse(tags, 0).getActiveObjects())
      ibc->computeResidual();

    for (auto nbc : _kokkos_nodal_bcs.getVectorTagsObjectWarehouse(tags, 0).getActiveObjects())
      nbc->computeResidual();
  }

  // Close and restore vectors

  {
    TIME_SECTION("KokkosCopy", 1);

    for (auto & system : systems)
      system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveResidualTags();

  for (auto & system : systems)
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }
}

void
NonlinearSystemBase::computeKokkosJacobian(const std::set<TagID> & tags)
{
  TIME_SECTION("computeKokkosJacobian", 1);

  auto & systems = _fe_problem.getKokkosSystems();

  systems[number()].setActiveMatrixTags(tags);

  // Resolve dependencies

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  _kokkos_kernels.updateVariableDependency(needed_moose_vars);
  _kokkos_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  _kokkos_kernels.updateMatPropDependency(needed_mat_props);

  _kokkos_integrated_bcs.updateVariableDependency(needed_moose_vars);
  _kokkos_integrated_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  _kokkos_integrated_bcs.updateMatPropDependency(needed_mat_props);

  _kokkos_nodal_bcs.updateVariableDependency(needed_moose_vars);
  _kokkos_nodal_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  // Copy data and cache variable values at element quadature points

  for (auto & system : systems)
  {
    system.setActiveVariables(needed_moose_vars);
    system.setActiveSolutionTags(needed_fe_var_vector_tags);

    {
      TIME_SECTION("KokkosPreallocate", 1);
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
    TIME_SECTION("KokkosKernel", 1);

    // Compute kernels

    for (auto kernel : _kokkos_kernels.getMatrixTagsObjectWarehouse(tags, 0).getActiveObjects())
      kernel->computeJacobian();

    for (auto nodal_kernel :
         _kokkos_nodal_kernels.getMatrixTagsObjectWarehouse(tags, 0).getActiveObjects())
      nodal_kernel->computeJacobian();

    for (auto ibc : _kokkos_integrated_bcs.getMatrixTagsObjectWarehouse(tags, 0).getActiveObjects())
      ibc->computeJacobian();

    for (auto nbc : _kokkos_nodal_bcs.getMatrixTagsObjectWarehouse(tags, 0).getActiveObjects())
      nbc->computeJacobian();
  }

  // Close and restore vectors and matrices

  {
    TIME_SECTION("KokkosCopy", 1);

    for (auto & system : systems)
      system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveMatrixTags();

  for (auto & system : systems)
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }
}
