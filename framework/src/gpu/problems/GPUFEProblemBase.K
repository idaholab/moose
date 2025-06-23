//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUTypes.h"

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "MaterialBase.h"

void
FEProblemBase::initGPU()
{
  // Initialize GPU assembly

  _gpu_assembly.init();
  _gpu_assembly.initQuadrature();
  _gpu_assembly.initShape();

  // Initialize GPU systems

  unsigned int max_system_number = 0;

  for (unsigned int s = 0; s < numNonlinearSystems(); ++s)
    max_system_number = std::max(max_system_number, getNonlinearSystemBase(s).number());

  max_system_number = std::max(max_system_number, getAuxiliarySystem().number());

  // GPUSystem does not have a default constructor, so this simply allocates buffer to which we
  // should do placement new later
  _gpu_systems.create(max_system_number + 1);

  for (unsigned int s = 0; s < numNonlinearSystems(); ++s)
  {
    auto & nl = getNonlinearSystemBase(s);
    new (&_gpu_systems[nl.number()]) GPUSystem(nl);
    _gpu_systems[nl.number()].init();
  }

  auto & aux = getAuxiliarySystem();
  new (&_gpu_systems[aux.number()]) GPUSystem(aux);
  _gpu_systems[aux.number()].init();

  // Initialize GPU material properties

  _gpu_material_props.allocateGPUProperties();
  _gpu_bnd_material_props.allocateGPUProperties();
  _gpu_neighbor_material_props.allocateGPUProperties();
}

void
FEProblemBase::initGPUStatefulProps()
{
  // Resolve dependencies

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;

  for (auto block : _mesh.meshSubdomains())
  {
    _gpu_materials.updateBlockVariableDependency(block, needed_moose_vars);
    _gpu_materials.updateBlockFEVariableCoupledVectorTagDependency(block,
                                                                   needed_fe_var_vector_tags);
  }

  // Copy data and preallocate quadature point solution vectors

  for (auto & system : _gpu_systems)
    if (system.initialized())
    {
      system.setActiveVariables(needed_moose_vars);
      system.setActiveVariableTags(needed_fe_var_vector_tags);
      system.sync(GPUMemcpyKind::HOST_TO_DEVICE);
      system.preallocate();
    }

  _gpu_systems.copy();

  // Initialize stateful properties

  for (auto & material : _gpu_materials.getActiveObjects())
    if (!material->hasRestoredProperties() && !material->boundaryRestricted())
      material->initStatefulProperties(0);

  for (auto & material : _gpu_materials[Moose::FACE_MATERIAL_DATA].getActiveObjects())
    if (!material->hasRestoredProperties())
      material->initStatefulProperties(0);

  for (auto & material : _gpu_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveObjects())
    if (!material->hasRestoredProperties())
      material->initStatefulProperties(0);

  for (auto & material : _gpu_materials.getActiveObjects())
    if (!material->hasRestoredProperties() && material->boundaryRestricted())
      material->initStatefulProperties(0);

  // Copy to old and older states

  _gpu_material_props.copy();
  _gpu_bnd_material_props.copy();
  _gpu_neighbor_material_props.copy();

  // Clear

  for (auto & system : _gpu_systems)
    if (system.initialized())
    {
      system.clearActiveVariables();
      system.clearActiveVariableTags();
      system.clearProjectionFlags();
    }
}

void
FEProblemBase::prepareGPUMaterials(
    const std::unordered_set<unsigned int> & consumer_needed_mat_props)
{
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto block : mesh().meshSubdomains())
    _gpu_materials.updateBlockMatPropDependency(block, needed_mat_props);

  for (auto boundary : mesh().meshBoundaryIds())
    _gpu_materials.updateBoundaryMatPropDependency(boundary, needed_mat_props);

  needed_mat_props.insert(consumer_needed_mat_props.begin(), consumer_needed_mat_props.end());

  setActiveMaterialProperties(needed_mat_props, 0);
}

void
FEProblemBase::reinitGPUMaterials()
{
  if (hasActiveMaterialProperties(0))
  {
    for (auto & material : _gpu_materials.getActiveObjects())
      if (!material->boundaryRestricted())
        material->computeProperties();

    for (auto & material : _gpu_materials[Moose::FACE_MATERIAL_DATA].getActiveObjects())
      material->computeProperties();

    for (auto & material : _gpu_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveObjects())
      material->computeProperties();

    for (auto & material : _gpu_materials.getActiveObjects())
      if (material->boundaryRestricted())
        material->computeProperties();
  }

  Kokkos::fence();
}
