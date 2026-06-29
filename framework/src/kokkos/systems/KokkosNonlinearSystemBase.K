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
  auto & systems = _fe_problem.getKokkosFESystems();

  auto tag = _fe_problem.addVectorTag("parallel_solution", Moose::VECTOR_TAG_SOLUTION);

  associateVectorToTag(solution(), tag);

  std::set<TagID> needed_fe_var_vector_tags = {tag};

  for (auto boundary : _fe_problem.mesh().meshBoundaryIds())
    _kokkos_preset_nodal_bcs.updateBoundaryFEVariableCoupledVectorTagDependency(
        boundary, needed_fe_var_vector_tags);

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
      systems[i].sync(needed_fe_var_vector_tags, Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);

  systems.copyToDevice();

  for (auto & nbc : _kokkos_preset_nodal_bcs.getActiveObjects())
    std::static_pointer_cast<Moose::Kokkos::NodalBCBase>(nbc)->presetSolution(tag);

  Kokkos::fence();

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
      systems[i].sync(needed_fe_var_vector_tags, Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

  disassociateVectorFromTag(solution(), tag);
}

void
NonlinearSystemBase::computeKokkosResidual(const std::set<TagID> & tags)
{
  TIME_SECTION("computeKokkosResidual", 1);

  // Get warehouses

  const auto & kernels = _kokkos_kernels.getVectorTagsObjectWarehouse(tags, 0);
  const auto & nodal_kernels = _kokkos_nodal_kernels.getVectorTagsObjectWarehouse(tags, 0);
  const auto & integrated_bcs = _kokkos_integrated_bcs.getVectorTagsObjectWarehouse(tags, 0);

  if (!kernels.size() && !nodal_kernels.size() && !integrated_bcs.size())
    return;

  // Resolve dependencies

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveResidualTags(tags);

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  kernels.updateVariableDependency(needed_moose_vars);
  kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  kernels.updateMatPropDependency(needed_mat_props);

  nodal_kernels.updateVariableDependency(needed_moose_vars);
  nodal_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  integrated_bcs.updateVariableDependency(needed_moose_vars);
  integrated_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  integrated_bcs.updateMatPropDependency(needed_mat_props);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  // Copy solution vectors and residuals and cache variable values at element quadature
  // points

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
    {
      auto & system = systems[i];
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

    for (auto & kernel : kernels.getActiveObjects())
      kernel->computeResidual();

    for (auto & nodal_kernel : nodal_kernels.getActiveObjects())
      nodal_kernel->computeResidual();

    for (auto & ibc : integrated_bcs.getActiveObjects())
      ibc->computeResidual();
  }

  // Close and restore vectors

  {
    TIME_SECTION("KokkosClose", 1);

    for (const auto i : index_range(systems))
      if (systems.isSlotConstructed(i))
        systems[i].sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveResidualTags();

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
    {
      auto & system = systems[i];
      system.clearActiveVariables();
      system.clearActiveSolutionTags();
    }
}

void
NonlinearSystemBase::computeKokkosNodalBCsResidual(const std::set<TagID> & tags)
{
  TIME_SECTION("computeKokkosNodalBCsResidual", 1);

  // Get warehouses

  const auto & nodal_bcs = _kokkos_nodal_bcs.getVectorTagsObjectWarehouse(tags, 0);

  if (!nodal_bcs.hasActiveObjects())
    return;

  // Resolve dependencies

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveResidualTags(tags);

  std::set<TagID> needed_fe_var_vector_tags;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  nodal_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  // Copy solution vectors and residuals

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
    {
      systems[i].setActiveSolutionTags(needed_fe_var_vector_tags);
      {
        TIME_SECTION("KokkosCopy", 1);
        systems[i].sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
      }
    }

  systems.copyToDevice();

  {
    TIME_SECTION("KokkosKernel", 1);

    // Compute kernels

    for (auto & nbc : nodal_bcs.getActiveObjects())
      nbc->computeResidual();
  }

  // Close and restore vectors

  {
    TIME_SECTION("KokkosClose", 1);

    for (const auto i : index_range(systems))
      if (systems.isSlotConstructed(i))
        systems[i].sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveResidualTags();

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
      systems[i].clearActiveSolutionTags();
}

void
NonlinearSystemBase::computeKokkosJacobian(const std::set<TagID> & tags)
{
  TIME_SECTION("computeKokkosJacobian", 1);

  // Get warehouses

  const auto & kernels = _kokkos_kernels.getMatrixTagsObjectWarehouse(tags, 0);
  const auto & nodal_kernels = _kokkos_nodal_kernels.getMatrixTagsObjectWarehouse(tags, 0);
  const auto & integrated_bcs = _kokkos_integrated_bcs.getMatrixTagsObjectWarehouse(tags, 0);
  const auto & nodal_bcs = _kokkos_nodal_bcs.getMatrixTagsObjectWarehouse(tags, 0);

  if (!kernels.hasActiveObjects() && !nodal_kernels.hasActiveObjects() &&
      !integrated_bcs.hasActiveObjects() && !nodal_bcs.hasActiveObjects())
    return;

  // Resolve dependencies

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveMatrixTags(tags);

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  kernels.updateVariableDependency(needed_moose_vars);
  kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  kernels.updateMatPropDependency(needed_mat_props);

  nodal_kernels.updateVariableDependency(needed_moose_vars);
  nodal_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  integrated_bcs.updateVariableDependency(needed_moose_vars);
  integrated_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  integrated_bcs.updateMatPropDependency(needed_mat_props);

  nodal_bcs.updateVariableDependency(needed_moose_vars);
  nodal_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  // Copy solution vectors, initialize matrices, and cache variable values at element quadature
  // points

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
    {
      auto & system = systems[i];
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

    for (auto & kernel : kernels.getActiveObjects())
      kernel->computeJacobian();

    for (auto & nodal_kernel : nodal_kernels.getActiveObjects())
      nodal_kernel->computeJacobian();

    for (auto & ibc : integrated_bcs.getActiveObjects())
      ibc->computeJacobian();

    for (auto & nbc : nodal_bcs.getActiveObjects())
      nbc->computeJacobian();
  }

  // Close and restore vectors and matrices

  {
    TIME_SECTION("KokkosClose", 1);

    for (const auto i : index_range(systems))
      if (systems.isSlotConstructed(i))
        systems[i].sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveMatrixTags();

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
    {
      auto & system = systems[i];
      system.clearActiveVariables();
      system.clearActiveSolutionTags();
    }
}

void
NonlinearSystemBase::computeKokkosResidualAndJacobian(const std::set<TagID> & vector_tags,
                                                      const std::set<TagID> & matrix_tags)
{
  TIME_SECTION("computeKokkosResidualAndJacobian", 1);

  if (!_kokkos_kernels.hasActiveObjects() && !_kokkos_nodal_kernels.hasActiveObjects() &&
      !_kokkos_integrated_bcs.hasActiveObjects() && !_kokkos_nodal_bcs.hasActiveObjects())
    return;

  // Resolve dependencies

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveResidualTags(vector_tags);
  systems[number()].setActiveMatrixTags(matrix_tags);

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  _kokkos_kernels.updateVariableDependency(needed_moose_vars);
  _kokkos_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  _kokkos_kernels.updateMatPropDependency(needed_mat_props);

  _kokkos_nodal_kernels.updateVariableDependency(needed_moose_vars);
  _kokkos_nodal_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

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

  // Copy solution vectors and residuals, initialize matrices, and cache variable values at element
  // quadature points

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
    {
      auto & system = systems[i];
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

    for (auto & kernel : _kokkos_kernels.getActiveObjects())
      kernel->computeResidualAndJacobian();

    for (auto & nodal_kernel : _kokkos_nodal_kernels.getActiveObjects())
      nodal_kernel->computeResidualAndJacobian();

    for (auto & ibc : _kokkos_integrated_bcs.getActiveObjects())
      ibc->computeResidualAndJacobian();

    /// Nodal BC residuals are computed separately
    for (auto & nbc : _kokkos_nodal_bcs.getActiveObjects())
      nbc->computeJacobian();
  }

  // Close and restore vectors and matrices

  {
    TIME_SECTION("KokkosClose", 1);

    for (const auto i : index_range(systems))
      if (systems.isSlotConstructed(i))
        systems[i].sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveResidualTags();
  systems[number()].clearActiveMatrixTags();

  for (const auto i : index_range(systems))
    if (systems.isSlotConstructed(i))
    {
      auto & system = systems[i];
      system.clearActiveVariables();
      system.clearActiveSolutionTags();
    }
}
