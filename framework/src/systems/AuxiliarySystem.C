//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "Factory.h"
#include "AuxKernel.h"
#include "AuxScalarKernel.h"
#include "MaterialData.h"
#include "Assembly.h"
#include "GeometricSearchData.h"
#include "ComputeNodalAuxVarsThread.h"
#include "ComputeNodalAuxBcsThread.h"
#include "ComputeElemAuxVarsThread.h"
#include "ComputeElemAuxBcsThread.h"
#include "ComputeMortarNodalAuxBndThread.h"
#include "ComputeLinearFVGreenGaussGradientFaceThread.h"
#include "ComputeLinearFVGreenGaussGradientVolumeThread.h"
#include "ComputeLinearFVLimitedGradientThread.h"
#include "Parser.h"
#include "TimeIntegrator.h"
#include "Conversion.h"

#include "libmesh/quadrature_gauss.h"
#include "libmesh/node_range.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/default_coupling.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe_interface.h"

// C++
#include <cstring> // for "Jacobian" exception test

using namespace libMesh;

// AuxiliarySystem ////////

AuxiliarySystem::AuxiliarySystem(FEProblemBase & subproblem, const std::string & name)
  : SystemBase(subproblem, subproblem, name, Moose::VAR_AUXILIARY),
    PerfGraphInterface(subproblem.getMooseApp().perfGraph(), "AuxiliarySystem"),
    _sys(subproblem.es().add_system<System>(name)),
    _current_solution(_sys.current_local_solution.get()),
    _aux_scalar_storage(_app.getExecuteOnEnum()),
    _nodal_aux_storage(_app.getExecuteOnEnum()),
    _mortar_nodal_aux_storage(_app.getExecuteOnEnum()),
    _elemental_aux_storage(_app.getExecuteOnEnum()),
    _nodal_vec_aux_storage(_app.getExecuteOnEnum()),
    _elemental_vec_aux_storage(_app.getExecuteOnEnum()),
    _nodal_array_aux_storage(_app.getExecuteOnEnum()),
    _elemental_array_aux_storage(_app.getExecuteOnEnum())
#ifdef MOOSE_KOKKOS_ENABLED
    ,
    _kokkos_nodal_aux_storage(_app.getExecuteOnEnum()),
    _kokkos_elemental_aux_storage(_app.getExecuteOnEnum())
#endif
{
  _nodal_vars.resize(libMesh::n_threads());
  _elem_vars.resize(libMesh::n_threads());

  if (!_fe_problem.defaultGhosting())
  {
    auto & dof_map = _sys.get_dof_map();
    dof_map.remove_algebraic_ghosting_functor(dof_map.default_algebraic_ghosting());
    dof_map.set_implicit_neighbor_dofs(false);
  }
}

AuxiliarySystem::~AuxiliarySystem() = default;

const std::vector<std::unique_ptr<NumericVector<Number>>> &
AuxiliarySystem::linearFVGradientContainer() const
{
  return _raw_grad_container;
}

void
AuxiliarySystem::requestLinearFVLimitedGradients(const Moose::FV::GradientLimiterType limiter_type)
{
  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return;

  if (_requested_limited_gradient_types.insert(limiter_type).second && !_raw_grad_container.empty())
  {
    const auto initialize_container =
        [this](std::vector<std::unique_ptr<NumericVector<Number>>> & container)
    {
      container.clear();
      for (const auto i : make_range(this->_mesh.dimension()))
      {
        libmesh_ignore(i);
        container.push_back(currentSolution()->zero_clone());
      }
    };

    initialize_container(_raw_limited_grad_containers[limiter_type]);
    initialize_container(_temporary_limited_gradient[limiter_type]);
  }
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
AuxiliarySystem::linearFVLimitedGradientContainer(
    const Moose::FV::GradientLimiterType limiter_type) const
{
  if (limiter_type != Moose::FV::GradientLimiterType::None)
  {
    const auto it = _raw_limited_grad_containers.find(limiter_type);
    if (it == _raw_limited_grad_containers.end())
      mooseError(
          "Limited gradient container was requested but not initialized on system '", name(), "'.");

    return it->second;
  }

  return _raw_grad_container;
}

const std::unordered_set<Moose::FV::GradientLimiterType> &
AuxiliarySystem::requestedLinearFVLimitedGradientTypes() const
{
  return _requested_limited_gradient_types;
}

void
AuxiliarySystem::initialSetup()
{
  TIME_SECTION("initialSetup", 3, "Initializing Auxiliary System");

  SystemBase::initialSetup();
  _current_solution = _sys.current_local_solution.get();
  rebuildLinearFVGradientStorage();

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.sort(tid);
    _aux_scalar_storage.initialSetup(tid);

    _nodal_aux_storage.sort(tid);
    _nodal_aux_storage.initialSetup(tid);

    _mortar_nodal_aux_storage.sort(tid);
    _mortar_nodal_aux_storage.initialSetup(tid);

    _nodal_vec_aux_storage.sort(tid);
    _nodal_vec_aux_storage.initialSetup(tid);

    _nodal_array_aux_storage.sort(tid);
    _nodal_array_aux_storage.initialSetup(tid);

    _elemental_aux_storage.sort(tid);
    _elemental_aux_storage.initialSetup(tid);

    _elemental_vec_aux_storage.sort(tid);
    _elemental_vec_aux_storage.initialSetup(tid);

    _elemental_array_aux_storage.sort(tid);
    _elemental_array_aux_storage.initialSetup(tid);
  }

#ifdef MOOSE_KOKKOS_ENABLED
  _kokkos_nodal_aux_storage.sort(/*tid=*/0);
  _kokkos_nodal_aux_storage.initialSetup(/*tid=*/0);

  _kokkos_elemental_aux_storage.sort(/*tid=*/0);
  _kokkos_elemental_aux_storage.initialSetup(/*tid=*/0);
#endif
}

void
AuxiliarySystem::reinit()
{
  _current_solution = _sys.current_local_solution.get();
  rebuildLinearFVGradientStorage();
}

void
AuxiliarySystem::rebuildLinearFVGradientStorage()
{
  _raw_grad_container.clear();
  _temporary_gradient.clear();
  _raw_limited_grad_containers.clear();
  _temporary_limited_gradient.clear();

  bool gradient_storage_initialized = false;
  for (const auto & field_var : _vars[0].fieldVariables())
    if (!gradient_storage_initialized && field_var->needsGradientVectorStorage())
      gradient_storage_initialized = true;

  if (!gradient_storage_initialized)
    return;

  const auto initialize_container =
      [this](std::vector<std::unique_ptr<NumericVector<Number>>> & container)
  {
    container.clear();
    for (const auto i : make_range(this->_mesh.dimension()))
    {
      libmesh_ignore(i);
      container.push_back(currentSolution()->zero_clone());
    }
  };

  initialize_container(_raw_grad_container);
  initialize_container(_temporary_gradient);

  for (const auto limiter_type : _requested_limited_gradient_types)
  {
    if (limiter_type == Moose::FV::GradientLimiterType::None)
      continue;

    initialize_container(_raw_limited_grad_containers[limiter_type]);
    initialize_container(_temporary_limited_gradient[limiter_type]);
  }
}

void
AuxiliarySystem::timestepSetup()
{
  SystemBase::timestepSetup();

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.timestepSetup(tid);
    _nodal_aux_storage.timestepSetup(tid);
    _mortar_nodal_aux_storage.timestepSetup(tid);
    _nodal_vec_aux_storage.timestepSetup(tid);
    _nodal_array_aux_storage.timestepSetup(tid);
    _elemental_aux_storage.timestepSetup(tid);
    _elemental_vec_aux_storage.timestepSetup(tid);
    _elemental_array_aux_storage.timestepSetup(tid);
  }

#ifdef MOOSE_KOKKOS_ENABLED
  _kokkos_nodal_aux_storage.timestepSetup(/*tid=*/0);
  _kokkos_elemental_aux_storage.timestepSetup(/*tid=*/0);
#endif
}

void
AuxiliarySystem::customSetup(const ExecFlagType & exec_type)
{
  SystemBase::customSetup(exec_type);

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.customSetup(exec_type, tid);
    _nodal_aux_storage.customSetup(exec_type, tid);
    _mortar_nodal_aux_storage.customSetup(exec_type, tid);
    _nodal_vec_aux_storage.customSetup(exec_type, tid);
    _nodal_array_aux_storage.customSetup(exec_type, tid);
    _elemental_aux_storage.customSetup(exec_type, tid);
    _elemental_vec_aux_storage.customSetup(exec_type, tid);
    _elemental_array_aux_storage.customSetup(exec_type, tid);
  }

#ifdef MOOSE_KOKKOS_ENABLED
  _kokkos_nodal_aux_storage.customSetup(exec_type, /*tid=*/0);
  _kokkos_elemental_aux_storage.customSetup(exec_type, /*tid=*/0);
#endif
}

void
AuxiliarySystem::subdomainSetup()
{
  SystemBase::subdomainSetup();

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.subdomainSetup(tid);
    _nodal_aux_storage.subdomainSetup(tid);
    _mortar_nodal_aux_storage.subdomainSetup(tid);
    _nodal_vec_aux_storage.subdomainSetup(tid);
    _nodal_array_aux_storage.subdomainSetup(tid);
    _elemental_aux_storage.subdomainSetup(tid);
    _elemental_vec_aux_storage.subdomainSetup(tid);
    _elemental_array_aux_storage.subdomainSetup(tid);
  }
}

void
AuxiliarySystem::jacobianSetup()
{
  SystemBase::jacobianSetup();

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.jacobianSetup(tid);
    _nodal_aux_storage.jacobianSetup(tid);
    _mortar_nodal_aux_storage.jacobianSetup(tid);
    _nodal_vec_aux_storage.jacobianSetup(tid);
    _nodal_array_aux_storage.jacobianSetup(tid);
    _elemental_aux_storage.jacobianSetup(tid);
    _elemental_vec_aux_storage.jacobianSetup(tid);
    _elemental_array_aux_storage.jacobianSetup(tid);
  }

#ifdef MOOSE_KOKKOS_ENABLED
  _kokkos_nodal_aux_storage.jacobianSetup(/*tid=*/0);
  _kokkos_elemental_aux_storage.jacobianSetup(/*tid=*/0);
#endif
}

void
AuxiliarySystem::residualSetup()
{
  SystemBase::residualSetup();

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.residualSetup(tid);
    _nodal_aux_storage.residualSetup(tid);
    _mortar_nodal_aux_storage.residualSetup(tid);
    _nodal_vec_aux_storage.residualSetup(tid);
    _nodal_array_aux_storage.residualSetup(tid);
    _elemental_aux_storage.residualSetup(tid);
    _elemental_vec_aux_storage.residualSetup(tid);
    _elemental_array_aux_storage.residualSetup(tid);
  }

#ifdef MOOSE_KOKKOS_ENABLED
  _kokkos_nodal_aux_storage.residualSetup(/*tid=*/0);
  _kokkos_elemental_aux_storage.residualSetup(/*tid=*/0);
#endif
}

void
AuxiliarySystem::updateActive(THREAD_ID tid)
{
  _aux_scalar_storage.updateActive(tid);
  _nodal_aux_storage.updateActive(tid);
  _mortar_nodal_aux_storage.updateActive(tid);
  _nodal_vec_aux_storage.updateActive(tid);
  _nodal_array_aux_storage.updateActive(tid);
  _elemental_aux_storage.updateActive(tid);
  _elemental_vec_aux_storage.updateActive(tid);
  _elemental_array_aux_storage.updateActive(tid);

#ifdef MOOSE_KOKKOS_ENABLED
  if (tid == 0)
  {
    _kokkos_nodal_aux_storage.updateActive(/*tid=*/0);
    _kokkos_elemental_aux_storage.updateActive(/*tid=*/0);
  }
#endif
}

void
AuxiliarySystem::addVariable(const std::string & var_type,
                             const std::string & name,
                             InputParameters & parameters)
{
  SystemBase::addVariable(var_type, name, parameters);

  auto fe_type = FEType(Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")),
                        Utility::string_to_enum<FEFamily>(parameters.get<MooseEnum>("family")));

  if (var_type == "MooseVariableScalar")
    return;

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    if (FEInterface::field_type(fe_type) == TYPE_VECTOR)
    {
      auto * var = _vars[tid].getActualFieldVariable<RealVectorValue>(name);
      if (var)
      {
        if (var->feType().family == LAGRANGE_VEC)
          _nodal_vars[tid].push_back(var);
        else
          _elem_vars[tid].push_back(var);
      }
    }

    else
    {
      MooseVariableBase * var_base = _vars[tid].getVariable(name);

      auto * const var = dynamic_cast<MooseVariableField<Real> *>(var_base);

      if (var)
      {
        if (var->feType().family == LAGRANGE)
          _nodal_vars[tid].push_back(var);
        else
          _elem_vars[tid].push_back(var);
      }

      auto * const avar = dynamic_cast<MooseVariableField<RealEigenVector> *>(var_base);

      if (avar)
      {
        if (avar->feType().family == LAGRANGE)
          _nodal_vars[tid].push_back(avar);
        else
          _elem_vars[tid].push_back(avar);
      }
    }
  }
}

void
AuxiliarySystem::addKernel(const std::string & kernel_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    const auto & base = parameters.getBase();
    if (base == "AuxKernel" || base == "Bounds")
    {
      std::shared_ptr<AuxKernel> kernel =
          _factory.create<AuxKernel>(kernel_name, name, parameters, tid);
      if (kernel->isNodal())
      {
        if (kernel->isMortar())
          _mortar_nodal_aux_storage.addObject(kernel, tid);
        else
          _nodal_aux_storage.addObject(kernel, tid);
      }
      else
        _elemental_aux_storage.addObject(kernel, tid);
    }

    else if (base == "VectorAuxKernel")
    {
      std::shared_ptr<VectorAuxKernel> kernel =
          _factory.create<VectorAuxKernel>(kernel_name, name, parameters, tid);
      if (kernel->isNodal())
      {
        if (kernel->isMortar())
          mooseError("Vector mortar aux kernels not yet implemented");
        _nodal_vec_aux_storage.addObject(kernel, tid);
      }
      else
        _elemental_vec_aux_storage.addObject(kernel, tid);
    }

    else if (base == "ArrayAuxKernel")
    {
      std::shared_ptr<ArrayAuxKernel> kernel =
          _factory.create<ArrayAuxKernel>(kernel_name, name, parameters, tid);
      if (kernel->isNodal())
      {
        if (kernel->isMortar())
          mooseError("Vector mortar aux kernels not yet implemented");
        _nodal_array_aux_storage.addObject(kernel, tid);
      }
      else
        _elemental_array_aux_storage.addObject(kernel, tid);
    }
    else
      mooseAssert(false,
                  "Attempting to add AuxKernel of type '" + kernel_name + "' and name '" + name +
                      "' to the auxiliary system with invalid _moose_base: " + base);
  }
}

void
AuxiliarySystem::addScalarKernel(const std::string & kernel_name,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<AuxScalarKernel> kernel =
        _factory.create<AuxScalarKernel>(kernel_name, name, parameters, tid);
    _aux_scalar_storage.addObject(kernel, tid);
  }
}

void
AuxiliarySystem::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{
  for (auto * var : _nodal_vars[tid])
    var->computeElemValues();

  for (auto * var : _elem_vars[tid])
  {
    var->reinitAux();
    var->computeElemValues();
  }
}

void
AuxiliarySystem::reinitElemFace(const Elem * /*elem*/, unsigned int /*side*/, THREAD_ID tid)
{
  for (auto * var : _nodal_vars[tid])
    var->computeElemValuesFace();

  for (auto * var : _elem_vars[tid])
  {
    var->reinitAux();
    var->reinitAuxNeighbor();
    var->computeElemValuesFace();
  }
}

void
AuxiliarySystem::serializeSolution()
{
  if (_serialized_solution.get() &&
      _sys.n_dofs() > 0) // libMesh does not like serializing of empty vectors
  {
    if (!_serialized_solution->initialized() || _serialized_solution->size() != _sys.n_dofs())
    {
      _serialized_solution->clear();
      _serialized_solution->init(_sys.n_dofs(), false, SERIAL);
    }

    solution().localize(*_serialized_solution);
  }
}

void
AuxiliarySystem::compute(ExecFlagType type)
{
  // avoid division by dt which might be zero.
  if (_fe_problem.dt() > 0.)
    for (auto & ti : _time_integrators)
      ti->preStep();

  // We need to compute time derivatives every time each kind of the variables is finished, because:
  //
  //  a) the user might want to use the aux variable value somewhere, thus we need to provide the
  //  up-to-date value
  //  b) time integration system works with the whole vectors of solutions, thus we cannot update
  //  only a part of the vector
  //

  if (_vars[0].scalars().size() > 0)
  {
    computeScalarVars(type);
    // compute time derivatives of scalar aux variables _after_ the values were updated
    if (_fe_problem.dt() > 0.)
      for (auto & ti : _time_integrators)
        ti->computeTimeDerivatives();
  }

  if (_vars[0].fieldVariables().size() > 0)
  {
    computeNodalArrayVars(type);
    computeNodalVecVars(type);
    computeNodalVars(type);
    computeMortarNodalVars(type);
    computeElementalArrayVars(type);
    computeElementalVecVars(type);
    computeElementalVars(type);

#ifdef MOOSE_KOKKOS_ENABLED
    kokkosCompute(type);
#endif

    if (!_raw_grad_container.empty())
    {
      solution().close();
      _sys.update();
      computeGradients();
    }

    // compute time derivatives of nodal aux variables _after_ the values were updated
    if (_fe_problem.dt() > 0.)
      for (auto & ti : _time_integrators)
        ti->computeTimeDerivatives();
  }

  if (_serialized_solution.get())
    serializeSolution();
}

void
AuxiliarySystem::computeGradients()
{
  if (_raw_grad_container.empty())
    return;

  auto & temporary_gradient = temporaryLinearFVGradientContainer();
  mooseAssert(temporary_gradient.size() == _raw_grad_container.size(),
              "Temporary and raw gradient containers must have the same size.");
  for (auto & vec : temporary_gradient)
    vec->zero();

  TIME_SECTION("LinearVariableFV_Gradients", 3 /*, "Computing Linear FV variable gradients"*/);

  PARALLEL_TRY
  {
    using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    FaceInfoRange face_info_range(_fe_problem.mesh().ownedFaceInfoBegin(),
                                  _fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        _fe_problem, *this, temporary_gradient);
    Threads::parallel_reduce(face_info_range, gradient_face_thread);
  }
  PARALLEL_CATCH;

  for (auto & vec : temporary_gradient)
    vec->close();

  PARALLEL_TRY
  {
    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(_fe_problem.mesh().ownedElemInfoBegin(),
                                  _fe_problem.mesh().ownedElemInfoEnd());

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        _fe_problem, *this, temporary_gradient);
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);
  }
  PARALLEL_CATCH;

  for (const auto i : index_range(_raw_grad_container))
    temporary_gradient[i]->close();

  _raw_grad_container.swap(temporary_gradient);

  if (!requestedLinearFVLimitedGradientTypes().empty())
  {
    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(_fe_problem.mesh().ownedElemInfoBegin(),
                                  _fe_problem.mesh().ownedElemInfoEnd());

    for (const auto limiter_type : requestedLinearFVLimitedGradientTypes())
    {
      if (limiter_type == Moose::FV::GradientLimiterType::None)
        continue;

      auto & raw_container = _raw_limited_grad_containers[limiter_type];
      auto & temporary_container = temporaryLinearFVLimitedGradientContainer(limiter_type);
      mooseAssert(temporary_container.size() == raw_container.size(),
                  "Temporary and raw limited gradient containers must have the same size.");
      for (auto & vec : temporary_container)
        vec->zero();

      PARALLEL_TRY
      {
        ComputeLinearFVLimitedGradientThread limited_gradient_thread(
            _fe_problem, *this, _raw_grad_container, temporary_container, limiter_type);
        Threads::parallel_reduce(elem_info_range, limited_gradient_thread);
      }
      PARALLEL_CATCH;

      for (auto & vec : temporary_container)
        vec->close();

      raw_container.swap(temporary_container);
    }
  }
}

std::set<std::string>
AuxiliarySystem::getDependObjects(ExecFlagType type)
{
  std::set<std::string> depend_objects;

  // Elemental AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs =
        _elemental_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Elemental VectorAuxKernels
  {
    const std::vector<std::shared_ptr<VectorAuxKernel>> & auxs =
        _elemental_vec_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Elemental ArrayAuxKernels
  {
    const std::vector<std::shared_ptr<ArrayAuxKernel>> & auxs =
        _elemental_array_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs =
        _nodal_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Mortar Nodal AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs =
        _mortar_nodal_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal VectorAuxKernels
  {
    const std::vector<std::shared_ptr<VectorAuxKernel>> & auxs =
        _nodal_vec_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal ArrayAuxKernels
  {
    const std::vector<std::shared_ptr<ArrayAuxKernel>> & auxs =
        _nodal_array_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

#ifdef MOOSE_KOKKOS_ENABLED
  // Kokkos NodalAuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernelBase>> & auxs =
        _kokkos_nodal_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Kokkos ElementalAuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernelBase>> & auxs =
        _kokkos_elemental_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }
#endif

  return depend_objects;
}

std::set<std::string>
AuxiliarySystem::getDependObjects()
{
  std::set<std::string> depend_objects;

  // Elemental AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs =
        _elemental_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Elemental VectorAuxKernels
  {
    const std::vector<std::shared_ptr<VectorAuxKernel>> & auxs =
        _elemental_vec_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Elemental ArrayAuxKernels
  {
    const std::vector<std::shared_ptr<ArrayAuxKernel>> & auxs =
        _elemental_array_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs = _nodal_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Mortar Nodal AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs =
        _mortar_nodal_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal VectorAuxKernels
  {
    const std::vector<std::shared_ptr<VectorAuxKernel>> & auxs =
        _nodal_vec_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal ArrayAuxKernels
  {
    const std::vector<std::shared_ptr<ArrayAuxKernel>> & auxs =
        _nodal_array_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

#ifdef MOOSE_KOKKOS_ENABLED
  // Nodal KokkosAuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernelBase>> & auxs =
        _kokkos_nodal_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Elemental KokkosAuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernelBase>> & auxs =
        _kokkos_elemental_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<UserObjectName> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }
#endif

  return depend_objects;
}

void
AuxiliarySystem::setScalarVariableCoupleableTags(ExecFlagType type)
{
  const MooseObjectWarehouse<AuxScalarKernel> & storage = _aux_scalar_storage[type];
  const std::vector<std::shared_ptr<AuxScalarKernel>> & objects = storage.getActiveObjects(0);

  std::set<TagID> needed_sc_var_matrix_tags;
  std::set<TagID> needed_sc_var_vector_tags;
  for (const auto & obj : objects)
  {
    auto & sc_var_coup_vtags = obj->getScalarVariableCoupleableVectorTags();
    needed_sc_var_vector_tags.insert(sc_var_coup_vtags.begin(), sc_var_coup_vtags.end());

    auto & sc_var_coup_mtags = obj->getScalarVariableCoupleableMatrixTags();
    needed_sc_var_matrix_tags.insert(sc_var_coup_mtags.begin(), sc_var_coup_mtags.end());
  }

  _fe_problem.setActiveScalarVariableCoupleableMatrixTags(needed_sc_var_matrix_tags, 0);
  _fe_problem.setActiveScalarVariableCoupleableVectorTags(needed_sc_var_vector_tags, 0);
}

void
AuxiliarySystem::clearScalarVariableCoupleableTags()
{
  _fe_problem.clearActiveScalarVariableCoupleableMatrixTags(0);
  _fe_problem.clearActiveScalarVariableCoupleableVectorTags(0);
}

void
AuxiliarySystem::computeScalarVars(ExecFlagType type)
{
  setScalarVariableCoupleableTags(type);

  // Reference to the current storage container
  const MooseObjectWarehouse<AuxScalarKernel> & storage = _aux_scalar_storage[type];

  if (storage.hasActiveObjects())
  {
    TIME_SECTION("computeScalarVars", 1);

    PARALLEL_TRY
    {
      // FIXME: run multi-threaded
      THREAD_ID tid = 0;
      if (storage.hasActiveObjects())
      {
        _fe_problem.reinitScalars(tid);

        const std::vector<std::shared_ptr<AuxScalarKernel>> & objects =
            storage.getActiveObjects(tid);

        // Call compute() method on all active AuxScalarKernel objects
        for (const auto & obj : objects)
          obj->compute();

        const std::vector<MooseVariableScalar *> & scalar_vars = getScalarVariables(tid);
        for (const auto & var : scalar_vars)
          var->insert(solution());
      }
    }
    PARALLEL_CATCH;

    solution().close();
    _sys.update();
  }

  clearScalarVariableCoupleableTags();
}

void
AuxiliarySystem::computeNodalVars(ExecFlagType type)
{
  TIME_SECTION("computeNodalVars", 3);

  const MooseObjectWarehouse<AuxKernel> & nodal = _nodal_aux_storage[type];
  computeNodalVarsHelper<AuxKernel>(nodal);
}

void
AuxiliarySystem::computeNodalVecVars(ExecFlagType type)
{
  TIME_SECTION("computeNodalVecVars", 3);

  const MooseObjectWarehouse<VectorAuxKernel> & nodal = _nodal_vec_aux_storage[type];
  computeNodalVarsHelper<VectorAuxKernel>(nodal);
}

void
AuxiliarySystem::computeNodalArrayVars(ExecFlagType type)
{
  const MooseObjectWarehouse<ArrayAuxKernel> & nodal = _nodal_array_aux_storage[type];
  computeNodalVarsHelper<ArrayAuxKernel>(nodal);
}

void
AuxiliarySystem::computeMortarNodalVars(const ExecFlagType type)
{
  TIME_SECTION("computeMortarNodalVars", 3);

  const MooseObjectWarehouse<AuxKernel> & mortar_nodal_warehouse = _mortar_nodal_aux_storage[type];

  mooseAssert(!mortar_nodal_warehouse.hasActiveBlockObjects(),
              "We don't allow creation of block restricted mortar nodal aux kernels.");

  if (mortar_nodal_warehouse.hasActiveBoundaryObjects())
  {
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (const auto & [bnd_id, mortar_nodal_auxes] :
         mortar_nodal_warehouse.getActiveBoundaryObjects())
      for (const auto index : index_range(mortar_nodal_auxes))
      {
        PARALLEL_TRY
        {
          try
          {
            ComputeMortarNodalAuxBndThread<AuxKernel> mnabt(
                _fe_problem, mortar_nodal_warehouse, bnd_id, index);
            Threads::parallel_reduce(bnd_nodes, mnabt);
          }
          catch (MooseException & e)
          {
            _fe_problem.setException("The following MooseException was raised during mortar nodal "
                                     "Auxiliary variable computation:\n" +
                                     std::string(e.what()));
          }
          catch (MetaPhysicL::LogicError & e)
          {
            moose::translateMetaPhysicLError(e);
          }
          catch (std::exception & e)
          {
            // Continue if we find a libMesh degenerate map exception, but
            // just re-throw for any real error
            if (!strstr(e.what(), "Jacobian") && !strstr(e.what(), "singular") &&
                !strstr(e.what(), "det != 0"))
              throw;

            _fe_problem.setException("We caught a libMesh degeneracy exception during mortar "
                                     "nodal Auxiliary variable computation:\n" +
                                     std::string(e.what()));
          }
        }
        PARALLEL_CATCH;

        // We need to make sure we propagate exceptions to all processes before trying to close
        // here, which is a parallel operation
        solution().close();
        _sys.update();
      }
  }
}

void
AuxiliarySystem::computeElementalVars(ExecFlagType type)
{
  TIME_SECTION("computeElementalVars", 3);

  const MooseObjectWarehouse<AuxKernel> & elemental = _elemental_aux_storage[type];
  computeElementalVarsHelper<AuxKernel>(elemental);
}

void
AuxiliarySystem::computeElementalVecVars(ExecFlagType type)
{
  TIME_SECTION("computeElementalVecVars", 3);

  const MooseObjectWarehouse<VectorAuxKernel> & elemental = _elemental_vec_aux_storage[type];
  computeElementalVarsHelper<VectorAuxKernel>(elemental);
}

void
AuxiliarySystem::computeElementalArrayVars(ExecFlagType type)
{
  const MooseObjectWarehouse<ArrayAuxKernel> & elemental = _elemental_array_aux_storage[type];
  computeElementalVarsHelper<ArrayAuxKernel>(elemental);
}

void
AuxiliarySystem::augmentSparsity(SparsityPattern::Graph & /*sparsity*/,
                                 std::vector<dof_id_type> & /*n_nz*/,
                                 std::vector<dof_id_type> &
                                 /*n_oz*/)
{
}

Order
AuxiliarySystem::getMinQuadratureOrder()
{
  Order order = CONSTANT;
  std::vector<MooseVariableFEBase *> vars = _vars[0].fieldVariables();
  for (const auto & var : vars)
  {
    if (!var->isNodal()) // nodal aux variables do not need quadrature
    {
      FEType fe_type = var->feType();
      if (fe_type.default_quadrature_order() > order)
        order = fe_type.default_quadrature_order();
    }
  }

  return order;
}

bool
AuxiliarySystem::needMaterialOnSide(BoundaryID bnd_id)
{
  return _elemental_aux_storage.hasActiveBoundaryObjects(bnd_id) ||
         _elemental_vec_aux_storage.hasActiveBoundaryObjects(bnd_id);
}

void
AuxiliarySystem::copyCurrentIntoPreviousNL()
{
  if (solutionPreviousNewton())
    *solutionPreviousNewton() = *currentSolution();
}

template <typename AuxKernelType>
void
AuxiliarySystem::computeElementalVarsHelper(const MooseObjectWarehouse<AuxKernelType> & warehouse)
{
  if (warehouse.hasActiveBlockObjects())
  {
    // Block Elemental AuxKernels
    PARALLEL_TRY
    {
      ConstElemRange & range = *_mesh.getActiveLocalElementRange();
      ComputeElemAuxVarsThread<AuxKernelType> eavt(_fe_problem, warehouse, true);
      try
      {
        Threads::parallel_reduce(range, eavt);
      }
      catch (MooseException & e)
      {
        _fe_problem.setException("The following MooseException was raised during elemental "
                                 "Auxiliary variable computation:\n" +
                                 std::string(e.what()));
      }
    }
    PARALLEL_CATCH;

    // We need to make sure we propagate exceptions to all processes before trying to close
    // here, which is a parallel operation
    solution().close();
    _sys.update();
  }

  // Boundary Elemental AuxKernels
  if (warehouse.hasActiveBoundaryObjects())
  {
    TIME_SECTION("computeElementalVecVars", 3);

    PARALLEL_TRY
    {
      ConstBndElemRange & bnd_elems = *_mesh.getBoundaryElementRange();
      ComputeElemAuxBcsThread<AuxKernelType> eabt(_fe_problem, warehouse, true);
      try
      {
        Threads::parallel_reduce(bnd_elems, eabt);
      }
      catch (MooseException & e)
      {
        _fe_problem.setException("The following MooseException was raised during boundary "
                                 "elemental Auxiliary variable computation:\n" +
                                 std::string(e.what()));
      }
    }
    PARALLEL_CATCH;

    // We need to make sure we propagate exceptions to all processes before trying to close
    // here, which is a parallel operation
    solution().close();
    _sys.update();
  }
}

template <typename AuxKernelType>
void
AuxiliarySystem::computeNodalVarsHelper(const MooseObjectWarehouse<AuxKernelType> & warehouse)
{
  if (warehouse.hasActiveBlockObjects())
  {
    // Block Nodal AuxKernels
    PARALLEL_TRY
    {
      ConstNodeRange & range = *_mesh.getLocalNodeRange();
      ComputeNodalAuxVarsThread<AuxKernelType> navt(_fe_problem, warehouse);
      Threads::parallel_reduce(range, navt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
  }

  if (warehouse.hasActiveBoundaryObjects())
  {
    TIME_SECTION("computeBoundaryObjects", 3);

    // Boundary Nodal AuxKernels
    PARALLEL_TRY
    {
      ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
      ComputeNodalAuxBcsThread<AuxKernelType> nabt(_fe_problem, warehouse);
      Threads::parallel_reduce(bnd_nodes, nabt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
  }
}

void
AuxiliarySystem::variableWiseRelativeSolutionDifferenceNorm(
    std::vector<Number> & rel_diff_norms) const
{
  rel_diff_norms.resize(nVariables(), 0);
  // Get dof map from system
  const auto & dof_map = _sys.get_dof_map();

  for (const auto n : make_range(nVariables()))
  {
    // Get local indices from dof map for each variable
    std::vector<dof_id_type> local_indices_n;
    dof_map.local_variable_indices(local_indices_n, _mesh, n);
    Number diff_norm_n = 0;
    Number norm_n = 0;
    // Get values from system, update norm
    for (const auto local_index : local_indices_n)
    {
      const Number & value = solution()(local_index);
      const Number & value_old = solutionOld()(local_index);
      diff_norm_n += Utility::pow<2, Number>(value - value_old);
      norm_n += Utility::pow<2, Number>(value);
    }
    // Aggregate norm over proceccors
    _communicator.sum(diff_norm_n);
    _communicator.sum(norm_n);
    diff_norm_n = sqrt(diff_norm_n);
    norm_n = sqrt(norm_n);
    rel_diff_norms[n] = diff_norm_n > 0 ? diff_norm_n / norm_n : 0.0;
  }
}

template void
AuxiliarySystem::computeElementalVarsHelper<AuxKernel>(const MooseObjectWarehouse<AuxKernel> &);
template void AuxiliarySystem::computeElementalVarsHelper<VectorAuxKernel>(
    const MooseObjectWarehouse<VectorAuxKernel> &);
template void
AuxiliarySystem::computeNodalVarsHelper<AuxKernel>(const MooseObjectWarehouse<AuxKernel> &);
template void AuxiliarySystem::computeNodalVarsHelper<VectorAuxKernel>(
    const MooseObjectWarehouse<VectorAuxKernel> &);
