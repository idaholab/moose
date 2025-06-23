//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUKernelBase.h"
#include "GPUNodalKernelBase.h"
#include "GPUNodalBCBase.h"
#include "GPUIntegratedBCBase.h"

#include "MaterialBase.h"
#include "NonlinearSystemBase.h"
#include "MooseVariableFieldBase.h"
#include "FEProblemBase.h"

void
NonlinearSystemBase::addGPUKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  // Create the kernel object via the factory and add to warehouse
  std::shared_ptr<GPUKernelBase> kernel =
      _factory.create<GPUKernelBase>(kernel_name, name, parameters);
  _gpu_kernels.addObject(kernel, 0);
  postAddResidualObject(*kernel);
}

void
NonlinearSystemBase::addGPUNodalKernel(const std::string & kernel_name,
                                       const std::string & name,
                                       InputParameters & parameters)
{
  // Create the kernel object via the factory and add to warehouse
  std::shared_ptr<GPUNodalKernelBase> kernel =
      _factory.create<GPUNodalKernelBase>(kernel_name, name, parameters);
  _gpu_nodal_kernels.addObject(kernel, 0);
  postAddResidualObject(*kernel);
}

void
NonlinearSystemBase::addGPUBoundaryCondition(const std::string & bc_name,
                                             const std::string & name,
                                             InputParameters & parameters)
{
  // Create the object
  std::shared_ptr<GPUBoundaryCondition> bc =
      _factory.create<GPUBoundaryCondition>(bc_name, name, parameters, 0);
  postAddResidualObject(*bc);

  // Active BoundaryIDs for the object
  const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  auto bc_var = dynamic_cast<const MooseVariableFieldBase *>(&bc->variable());
  _vars[0].addBoundaryVar(boundary_ids, bc_var);

  // Cast to the various types of BCs
  std::shared_ptr<GPUNodalBCBase> nbc = std::dynamic_pointer_cast<GPUNodalBCBase>(bc);
  std::shared_ptr<GPUIntegratedBCBase> ibc = std::dynamic_pointer_cast<GPUIntegratedBCBase>(bc);

  // GPUNodalBCBase
  if (nbc)
  {
    if (!nbc->variable().isNodal())
      mooseError("Trying to use nodal boundary condition '",
                 nbc->name(),
                 "' on a non-nodal variable '",
                 nbc->variable().name(),
                 "'.");

    _gpu_nodal_bcs.addObject(nbc);
    _vars[0].addBoundaryVars(boundary_ids, nbc->getCoupledVars());

    // GPUDirichletBCs that are preset
    if (nbc->preset())
      _gpu_preset_nodal_bcs.addObject(nbc);
  }

  // GPUIntegratedBCBase
  else if (ibc)
  {
    _gpu_integrated_bcs.addObject(ibc);
    _vars[0].addBoundaryVars(boundary_ids, ibc->getCoupledVars());
  }

  else
    mooseError("Unknown GPUBoundaryCondition type for object named ", bc->name());
}

void
NonlinearSystemBase::setGPUInitialSolution()
{
  auto & systems = _fe_problem.getGPUSystems();

  auto tag = _fe_problem.addVectorTag("_solution", Moose::VECTOR_TAG_SOLUTION);

  associateVectorToTag(solution(), tag);

  std::set<TagID> needed_fe_var_vector_tags = {tag};

  for (auto boundary : _fe_problem.mesh().meshBoundaryIds())
    _gpu_nodal_bcs.updateBoundaryFEVariableCoupledVectorTagDependency(boundary,
                                                                      needed_fe_var_vector_tags);

  for (auto & system : systems)
    if (system.initialized())
      system.sync(needed_fe_var_vector_tags, GPUMemcpyKind::HOST_TO_DEVICE);

  systems.copy();

  for (auto nbc : _gpu_preset_nodal_bcs.getActiveObjects())
    std::static_pointer_cast<GPUNodalBCBase>(nbc)->presetSolution(tag);

  Kokkos::fence();

  systems[number()].sync({tag}, GPUMemcpyKind::DEVICE_TO_HOST);

  disassociateVectorFromTag(solution(), tag);
}

void
NonlinearSystemBase::computeGPUResidual(const std::set<TagID> & tags)
{
  TIME_SECTION("computeGPUResidual", 1);

  auto & systems = _fe_problem.getGPUSystems();

  systems[number()].setActiveResidualTags(tags);

  // Resolve dependencies

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  for (auto block : _fe_problem.mesh().meshSubdomains())
  {
    _fe_problem.getGPUMaterialsWarehouse().updateBlockVariableDependency(block, needed_moose_vars);
    _fe_problem.getGPUMaterialsWarehouse().updateBlockFEVariableCoupledVectorTagDependency(
        block, needed_fe_var_vector_tags);

    _gpu_kernels.updateBlockVariableDependency(block, needed_moose_vars);
    _gpu_kernels.updateBlockFEVariableCoupledVectorTagDependency(block, needed_fe_var_vector_tags);
    _gpu_kernels.updateBlockMatPropDependency(block, needed_mat_props);
  }

  for (auto boundary : _fe_problem.mesh().meshBoundaryIds())
  {
    _gpu_integrated_bcs.updateBoundaryVariableDependency(boundary, needed_moose_vars);
    _gpu_integrated_bcs.updateBoundaryFEVariableCoupledVectorTagDependency(
        boundary, needed_fe_var_vector_tags);

    _gpu_nodal_bcs.updateBoundaryVariableDependency(boundary, needed_moose_vars);
    _gpu_nodal_bcs.updateBoundaryFEVariableCoupledVectorTagDependency(boundary,
                                                                      needed_fe_var_vector_tags);
  }

  for (auto boundary : _fe_problem.mesh().meshBoundaryIds())
    _gpu_integrated_bcs.updateBoundaryMatPropDependency(boundary, needed_mat_props);

  // Copy data and preallocate quadature point solution vectors

  for (auto & system : systems)
    if (system.initialized())
    {
      system.setActiveVariables(needed_moose_vars);
      system.setActiveVariableTags(needed_fe_var_vector_tags);

      {
        TIME_SECTION("GPUCopy", 1);
        system.sync(GPUMemcpyKind::HOST_TO_DEVICE);
        system.preallocate();
      }
    }

  systems.copy();

  {
    TIME_SECTION("GPUMaterial", 1);

    // Compute material properties

    _fe_problem.prepareGPUMaterials(needed_mat_props);
    _fe_problem.reinitGPUMaterials();
  }

  {
    TIME_SECTION("GPUKernel", 1);

    // Compute kernels

    for (auto kernel : _gpu_kernels.getVectorTagsObjectWarehouse(tags, 0).getActiveObjects())
      kernel->computeResidual();

    for (auto nodal_kernel :
         _gpu_nodal_kernels.getVectorTagsObjectWarehouse(tags, 0).getActiveObjects())
      nodal_kernel->computeResidual();

    for (auto ibc : _gpu_integrated_bcs.getVectorTagsObjectWarehouse(tags, 0).getActiveObjects())
      ibc->computeResidual();

    for (auto nbc : _gpu_nodal_bcs.getVectorTagsObjectWarehouse(tags, 0).getActiveObjects())
      nbc->computeResidual();
  }

  // Copy back residual

  {
    TIME_SECTION("GPUCopy", 1);
    systems[number()].sync(GPUMemcpyKind::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveResidualTags();

  for (auto & system : systems)
    if (system.initialized())
    {
      system.clearActiveVariables();
      system.clearActiveVariableTags();
      system.clearProjectionFlags();
    }
}

void
NonlinearSystemBase::computeGPUJacobian(const std::set<TagID> & tags)
{
  TIME_SECTION("computeGPUJacobian", 1);

  auto & systems = _fe_problem.getGPUSystems();

  systems[number()].setActiveMatrixTags(tags);

  // Resolve dependencies

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  for (auto block : _fe_problem.mesh().meshSubdomains())
  {
    _fe_problem.getGPUMaterialsWarehouse().updateBlockVariableDependency(block, needed_moose_vars);
    _fe_problem.getGPUMaterialsWarehouse().updateBlockFEVariableCoupledVectorTagDependency(
        block, needed_fe_var_vector_tags);

    _gpu_kernels.updateBlockVariableDependency(block, needed_moose_vars);
    _gpu_kernels.updateBlockFEVariableCoupledVectorTagDependency(block, needed_fe_var_vector_tags);
    _gpu_kernels.updateBlockMatPropDependency(block, needed_mat_props);
  }

  for (auto boundary : _fe_problem.mesh().meshBoundaryIds())
  {
    _gpu_integrated_bcs.updateBoundaryVariableDependency(boundary, needed_moose_vars);
    _gpu_integrated_bcs.updateBoundaryFEVariableCoupledVectorTagDependency(
        boundary, needed_fe_var_vector_tags);

    _gpu_nodal_bcs.updateBoundaryVariableDependency(boundary, needed_moose_vars);
    _gpu_nodal_bcs.updateBoundaryFEVariableCoupledVectorTagDependency(boundary,
                                                                      needed_fe_var_vector_tags);
  }

  for (auto boundary : _fe_problem.mesh().meshBoundaryIds())
    _gpu_integrated_bcs.updateBoundaryMatPropDependency(boundary, needed_mat_props);

  // Copy data and preallocate quadature point solution vectors

  for (auto & system : systems)
    if (system.initialized())
    {
      system.setActiveVariables(needed_moose_vars);
      system.setActiveVariableTags(needed_fe_var_vector_tags);

      {
        TIME_SECTION("GPUPreallocate", 1);
        system.sync(GPUMemcpyKind::HOST_TO_DEVICE);
        system.preallocate();
      }
    }

  systems.copy();

  {
    TIME_SECTION("GPUMaterial", 1);

    // Compute material properties

    _fe_problem.prepareGPUMaterials(needed_mat_props);
    _fe_problem.reinitGPUMaterials();
  }

  {
    TIME_SECTION("GPUKernel", 1);

    // Compute kernels

    for (auto kernel : _gpu_kernels.getMatrixTagsObjectWarehouse(tags, 0).getActiveObjects())
      kernel->computeJacobian();

    for (auto nodal_kernel :
         _gpu_nodal_kernels.getMatrixTagsObjectWarehouse(tags, 0).getActiveObjects())
      nodal_kernel->computeJacobian();

    for (auto ibc : _gpu_integrated_bcs.getMatrixTagsObjectWarehouse(tags, 0).getActiveObjects())
      ibc->computeJacobian();

    for (auto nbc : _gpu_nodal_bcs.getMatrixTagsObjectWarehouse(tags, 0).getActiveObjects())
      nbc->computeJacobian();
  }

  // Copy back matrix

  {
    TIME_SECTION("GPUCopy", 1);
    systems[number()].sync(GPUMemcpyKind::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveMatrixTags();

  for (auto & system : systems)
    if (system.initialized())
    {
      system.clearActiveVariables();
      system.clearActiveVariableTags();
      system.clearProjectionFlags();
    }
}
