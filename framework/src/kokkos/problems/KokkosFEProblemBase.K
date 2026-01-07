//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTypes.h"
#include "KokkosFunction.h"

#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "MaterialBase.h"

Moose::Kokkos::System &
FEProblemBase::getKokkosSystem(const unsigned int sys_num)
{
  return _kokkos_systems[sys_num];
}

const Moose::Kokkos::System &
FEProblemBase::getKokkosSystem(const unsigned int sys_num) const
{
  return _kokkos_systems[sys_num];
}

void
FEProblemBase::addKokkosKernel(const std::string & kernel_name,
                               const std::string & name,
                               InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a Kernel to a linear variable/system, which is not "
               "supported at the moment!");

  setResidualObjectParamsAndLog(
      kernel_name, name, parameters, nl_sys_num, "KokkosKernel", _reinit_displaced_elem);

  _nl[nl_sys_num]->addKokkosKernel(kernel_name, name, parameters);

  _has_kokkos_objects = true;
  _has_kokkos_residual_objects = true;
}

void
FEProblemBase::addKokkosNodalKernel(const std::string & kernel_name,
                                    const std::string & name,
                                    InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;

  setResidualObjectParamsAndLog(
      kernel_name, name, parameters, nl_sys_num, "KokkosNodalKernel", _reinit_displaced_elem);

  _nl[nl_sys_num]->addKokkosNodalKernel(kernel_name, name, parameters);

  _has_kokkos_objects = true;
  _has_kokkos_residual_objects = true;
}

void
FEProblemBase::addKokkosBoundaryCondition(const std::string & bc_name,
                                          const std::string & name,
                                          InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError(
        "You are trying to add a BoundaryCondition to a linear variable/system, which is not "
        "supported at the moment!");

  setResidualObjectParamsAndLog(
      bc_name, name, parameters, nl_sys_num, "KokkosBoundaryCondition", _reinit_displaced_face);

  _nl[nl_sys_num]->addKokkosBoundaryCondition(bc_name, name, parameters);

  _has_kokkos_objects = true;
  _has_kokkos_residual_objects = true;
}

void
FEProblemBase::addKokkosAuxKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  parallel_object_only();

  setAuxKernelParamsAndLog(kernel_name, name, parameters, "KokkosAuxKernel");

  _aux->addKokkosKernel(kernel_name, name, parameters);

  _has_kokkos_objects = true;
}

void
FEProblemBase::addKokkosFunction(const std::string & type,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  parallel_object_only();

  parameters.set<SubProblem *>("_subproblem") = this;

  std::shared_ptr<Moose::FunctionBase> func =
      _factory.create<Moose::FunctionBase>(type, name, parameters);
  logAdd("Function", name, type, parameters);
  _kokkos_functions.addObject(func);

  _has_kokkos_objects = true;
}

bool
FEProblemBase::hasKokkosFunction(const std::string & name)
{
  return _kokkos_functions.hasActiveObject(name);
}

Moose::Kokkos::Function
FEProblemBase::getKokkosFunction(const std::string & name)
{
  const auto & function = getKokkosFunction<Moose::FunctionBase>(name);

  std::shared_ptr<Moose::Kokkos::FunctionWrapperHostBase> wrapper =
      Moose::Kokkos::FunctorRegistry::buildFunction(&function, function.type());

  return Moose::Kokkos::Function(wrapper);
}

void
FEProblemBase::addKokkosMaterial(const std::string & mat_name,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  addMaterialHelper({&_kokkos_materials}, mat_name, name, parameters);

  _has_kokkos_objects = true;
}

MaterialData &
FEProblemBase::getKokkosMaterialData(Moose::MaterialDataType type, const MooseObject * object) const
{
  switch (type)
  {
    case Moose::BLOCK_MATERIAL_DATA:
      if (object)
        _kokkos_material_props.addConsumer(type, object);
      return _kokkos_material_props.getMaterialData(0);
    case Moose::NEIGHBOR_MATERIAL_DATA:
      if (object)
        _kokkos_neighbor_material_props.addConsumer(type, object);
      return _kokkos_neighbor_material_props.getMaterialData(0);
    case Moose::BOUNDARY_MATERIAL_DATA:
    case Moose::FACE_MATERIAL_DATA:
    case Moose::INTERFACE_MATERIAL_DATA:
      if (object)
        _kokkos_bnd_material_props.addConsumer(type, object);
      return _kokkos_bnd_material_props.getMaterialData(0);
  }

  mooseError("FEProblemBase::getKokkosMaterialData(): Invalid MaterialDataType ", type);
}

const std::set<const MooseObject *> &
FEProblemBase::getKokkosMaterialPropertyStorageConsumers(Moose::MaterialDataType type) const
{
  switch (type)
  {
    case Moose::BLOCK_MATERIAL_DATA:
      return _kokkos_material_props.getConsumers(type);
    case Moose::NEIGHBOR_MATERIAL_DATA:
      return _kokkos_neighbor_material_props.getConsumers(type);
    case Moose::BOUNDARY_MATERIAL_DATA:
    case Moose::FACE_MATERIAL_DATA:
    case Moose::INTERFACE_MATERIAL_DATA:
      return _kokkos_bnd_material_props.getConsumers(type);
  }

  mooseError(
      "FEProblemBase::getKokkosMaterialPropertyStorageConsumers(): Invalid MaterialDataType ",
      type);
}

void
FEProblemBase::initKokkos()
{
  TIME_SECTION("InitializeKokkos", 1, "Initializing Kokkos");

  // Error on unsupported options

  if (haveDisplaced())
    mooseError("Kokkos does not support displaced mesh yet.");

  if (adaptivity().isOn())
    mooseError("Kokkos does not support adaptivity yet.");

  // Initialize Kokkos mesh
  _mesh.getKokkosMesh()->update();

  // Call functions that require Kokkos mesh to be initialized first
  for (auto hook : _kokkos_mesh_initialization_hooks)
    hook();

  // Initialize Kokkos assembly
  _kokkos_assembly.init();

  // Initialize Kokkos systems

  unsigned int max_system_number = 0;

  for (unsigned int s = 0; s < numNonlinearSystems(); ++s)
    max_system_number = std::max(max_system_number, getNonlinearSystemBase(s).number());

  max_system_number = std::max(max_system_number, getAuxiliarySystem().number());

  // Kokkos system does not have a default constructor, so this simply allocates buffer to which we
  // should do placement new later
  _kokkos_systems.create(max_system_number + 1);

  for (unsigned int s = 0; s < numNonlinearSystems(); ++s)
  {
    auto & nl = getNonlinearSystemBase(s);
    new (&_kokkos_systems[nl.number()]) Moose::Kokkos::System(nl);
  }

  auto & aux = getAuxiliarySystem();
  new (&_kokkos_systems[aux.number()]) Moose::Kokkos::System(aux);

  // Initialize Kokkos material properties

  _kokkos_material_props.allocateKokkosProperties();
  _kokkos_bnd_material_props.allocateKokkosProperties();
  _kokkos_neighbor_material_props.allocateKokkosProperties();
}

void
FEProblemBase::initKokkosStatefulProps()
{
  // Resolve dependencies

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;

  for (auto block : _mesh.meshSubdomains())
  {
    _kokkos_materials.updateBlockVariableDependency(block, needed_moose_vars);
    _kokkos_materials.updateBlockFEVariableCoupledVectorTagDependency(block,
                                                                      needed_fe_var_vector_tags);
  }

  for (auto boundary : _mesh.meshBoundaryIds())
  {
    _kokkos_materials.updateBoundaryVariableDependency(boundary, needed_moose_vars);
    _kokkos_materials.updateBoundaryFEVariableCoupledVectorTagDependency(boundary,
                                                                         needed_fe_var_vector_tags);
  }

  // Copy data and preallocate quadature point solution vectors

  for (auto & system : _kokkos_systems)
  {
    system.setActiveVariables(needed_moose_vars);
    system.setActiveSolutionTags(needed_fe_var_vector_tags);

    system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
    system.reinit();
  }

  _kokkos_systems.copyToDevice();

  // Initialize stateful properties

  for (auto & material : _kokkos_materials.getActiveObjects())
    if (!material->hasRestoredProperties() && !material->boundaryRestricted())
      material->initStatefulProperties(0);

  for (auto & material : _kokkos_materials[Moose::FACE_MATERIAL_DATA].getActiveObjects())
    if (!material->hasRestoredProperties())
      material->initStatefulProperties(0);

  for (auto & material : _kokkos_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveObjects())
    if (!material->hasRestoredProperties())
      material->initStatefulProperties(0);

  for (auto & material : _kokkos_materials.getActiveObjects())
    if (!material->hasRestoredProperties() && material->boundaryRestricted())
      material->initStatefulProperties(0);

  // Copy to old and older states

  _kokkos_material_props.copy();
  _kokkos_bnd_material_props.copy();
  _kokkos_neighbor_material_props.copy();

  // Clear

  for (auto & system : _kokkos_systems)
  {
    system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }
}

void
FEProblemBase::prepareKokkosMaterials(
    const std::unordered_set<unsigned int> & consumer_needed_mat_props)
{
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto block : mesh().meshSubdomains())
    _kokkos_materials.updateBlockMatPropDependency(block, needed_mat_props);

  for (auto boundary : mesh().meshBoundaryIds())
    _kokkos_materials.updateBoundaryMatPropDependency(boundary, needed_mat_props);

  needed_mat_props.insert(consumer_needed_mat_props.begin(), consumer_needed_mat_props.end());

  setActiveMaterialProperties(needed_mat_props, 0);
}

void
FEProblemBase::reinitKokkosMaterials()
{
  if (hasActiveMaterialProperties(0))
  {
    for (auto & material : _kokkos_materials.getActiveObjects())
      if (!material->boundaryRestricted())
        material->computeProperties();

    for (auto & material : _kokkos_materials[Moose::FACE_MATERIAL_DATA].getActiveObjects())
      material->computeProperties();

    for (auto & material : _kokkos_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveObjects())
      material->computeProperties();

    for (auto & material : _kokkos_materials.getActiveObjects())
      if (material->boundaryRestricted())
        material->computeProperties();
  }

  Kokkos::fence();
}
