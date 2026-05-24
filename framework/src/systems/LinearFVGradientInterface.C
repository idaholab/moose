//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGradientInterface.h"

#include "ComputeLinearFVGreenGaussGradientFaceThread.h"
#include "ComputeLinearFVGreenGaussGradientVolumeThread.h"
#include "ComputeLinearFVLimitedGradientThread.h"
#include "FEProblemBase.h"
#include "PerfGraphInterface.h"
#include "PerfGuard.h"
#include "SystemBase.h"
#include "MooseVariableFieldBase.h"
#include "MooseError.h"
#include "ElemInfo.h"

#include "libmesh/numeric_vector.h"

using namespace libMesh;

LinearFVGradientField::LinearFVGradientField(const SystemBase & sys,
                                             const GradientContainer & components)
  : _sys(sys), _components(components)
{
}

Real
LinearFVGradientField::component(const ElemInfo & elem_info,
                                 const unsigned int variable_number,
                                 const unsigned int component) const
{
  mooseAssert(component < _components.size(), "Gradient component index out of range.");

  return (*_components[component])(elem_info.dofIndices()[_sys.number()][variable_number]);
}

RealVectorValue
LinearFVGradientField::gradient(const ElemInfo & elem_info,
                                const unsigned int variable_number) const
{
  RealVectorValue value;
  value.zero();

  for (const auto component_index : make_range(_sys.mesh().dimension()))
    value(component_index) = component(elem_info, variable_number, component_index);

  return value;
}

const LinearFVGradientField &
LinearFVGradientInterface::linearFVGradientField(
    const Moose::FV::GradientLimiterType limiter_type) const
{
  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return _raw_gradient_field;

  const auto it = _limited_gradient_fields.find(limiter_type);
  if (it == _limited_gradient_fields.end())
    mooseError(
        "Limited gradient field was requested but not initialized on system '", _sys.name(), "'.");

  return *it->second;
}

LinearFVGradientField &
LinearFVGradientInterface::registerFVGradient(
    const unsigned int variable_number,
    const Moose::FV::LinearFVGradientSchemeType scheme_type,
    const Moose::FV::GradientLimiterType limiter_type)
{
  auto * const variable =
      dynamic_cast<MooseVariableFieldBase *>(_sys.variableWarehouse().getVariable(variable_number));
  if (!variable)
    mooseError("Linear FV gradients were requested for variable number ",
               variable_number,
               " on system '",
               _sys.name(),
               "', but no field variable with that number exists on the system.");

  const auto scheme_it = _registered_gradient_schemes.find(variable_number);
  if (scheme_it != _registered_gradient_schemes.end() && scheme_it->second != scheme_type)
    mooseError("Linear FV gradients were requested for variable '",
               variable->name(),
               "' on system '",
               _sys.name(),
               "' with multiple gradient schemes.");

  _registered_gradient_schemes[variable_number] = scheme_type;
  _registered_gradient_scheme_variables[scheme_type].insert(variable_number);

  if (_raw_grad_container.empty() && _sys.currentSolution())
  {
    initializeContainer(_raw_grad_container);
    initializeContainer(_temporary_gradient);
    for (const auto limiter_type : _requested_limited_gradient_types)
    {
      if (limiter_type == Moose::FV::GradientLimiterType::None)
        continue;

      initializeContainer(_raw_limited_grad_containers[limiter_type]);
      initializeContainer(_temporary_limited_gradient[limiter_type]);
      if (!_limited_gradient_fields.count(limiter_type))
        initializeLimitedGradientField(limiter_type);
    }
  }

  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return _raw_gradient_field;

  requestLinearFVLimitedGradients(limiter_type, variable_number);
  return *libmesh_map_find(_limited_gradient_fields, limiter_type);
}

void
LinearFVGradientInterface::computeGradients()
{
  // No gradients have been requested, by now we should have set up the
  // containers to receive the gradients. Time to early return.
  if (_raw_grad_container.empty())
    return;

  auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
  mooseAssert(perf_graph_interface,
              "LinearFVGradientInterface requires its owning system to implement "
              "PerfGraphInterface.");
  const auto perf_id = perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
  mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
  PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

  updateBaseGradientField();

  for (const auto limiter_type : requestedLinearFVLimitedGradientTypes())
    if (limiter_type != Moose::FV::GradientLimiterType::None)
      updateLimitedGradient(limiter_type);
}

void
LinearFVGradientInterface::updateFVGradient(const LinearFVGradientField & field)
{
  if (_raw_grad_container.empty())
    return;

  auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
  mooseAssert(perf_graph_interface,
              "LinearFVGradientInterface requires its owning system to implement "
              "PerfGraphInterface.");
  const auto perf_id = perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
  mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
  PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

  if (&field == &_raw_gradient_field)
  {
    updateBaseGradientField();
    return;
  }

  for (const auto & limiter_field_pair : _limited_gradient_fields)
    if (limiter_field_pair.second.get() == &field)
    {
      updateBaseGradientField();
      updateLimitedGradient(limiter_field_pair.first);
      return;
    }

  mooseError("Requested update for a linear FV gradient field that is not registered on system '",
             _sys.name(),
             "'.");
}

void
LinearFVGradientInterface::updateBaseGradientField()
{
  auto & temporary_gradient = temporaryLinearFVGradientContainer();
  mooseAssert(temporary_gradient.size() == _raw_grad_container.size(),
              "Temporary and raw gradient containers must have the same size.");
  for (auto & vec : temporary_gradient)
    vec->zero();

  auto & fe_problem = _sys.feProblem();
  const auto have_registered_gradient_variables = !_registered_gradient_schemes.empty();
  const std::unordered_set<unsigned int> empty_gradient_variables;
  const auto scheme_variables_it =
      _registered_gradient_scheme_variables.find(Moose::FV::LinearFVGradientSchemeType::GreenGauss);
  const auto & producer_gradient_variables =
      scheme_variables_it == _registered_gradient_scheme_variables.end()
          ? empty_gradient_variables
          : scheme_variables_it->second;

  PARALLEL_TRY
  {
    using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    FaceInfoRange face_info_range(fe_problem.mesh().ownedFaceInfoBegin(),
                                  fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        fe_problem,
        _sys,
        temporary_gradient,
        producer_gradient_variables,
        have_registered_gradient_variables);
    Threads::parallel_reduce(face_info_range, gradient_face_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (auto & vec : temporary_gradient)
    vec->close();

  PARALLEL_TRY
  {
    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(fe_problem.mesh().ownedElemInfoBegin(),
                                  fe_problem.mesh().ownedElemInfoEnd());

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        fe_problem,
        _sys,
        temporary_gradient,
        producer_gradient_variables,
        have_registered_gradient_variables);
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (const auto i : index_range(_raw_grad_container))
    temporary_gradient[i]->close();

  _raw_grad_container.swap(temporary_gradient);
}

void
LinearFVGradientInterface::updateLimitedGradient(const Moose::FV::GradientLimiterType limiter_type)
{
  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return;

  auto & fe_problem = _sys.feProblem();
  using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
  ElemInfoRange elem_info_range(fe_problem.mesh().ownedElemInfoBegin(),
                                fe_problem.mesh().ownedElemInfoEnd());

  auto & raw_container = rawLinearFVLimitedGradientContainer(limiter_type);
  auto & temporary_container = temporaryLinearFVLimitedGradientContainer(limiter_type);
  mooseAssert(temporary_container.size() == raw_container.size(),
              "Temporary and raw limited gradient containers must have the same size.");
  for (auto & vec : temporary_container)
    vec->zero();

  PARALLEL_TRY
  {
    ComputeLinearFVLimitedGradientThread limited_gradient_thread(
        fe_problem,
        _sys,
        _raw_grad_container,
        temporary_container,
        limiter_type,
        requestedLinearFVLimitedGradientVariables(limiter_type));
    Threads::parallel_reduce(elem_info_range, limited_gradient_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (auto & vec : temporary_container)
    vec->close();

  raw_container.swap(temporary_container);
}

bool
LinearFVGradientInterface::needsLinearFVGradientStorage() const
{
  if (!_registered_gradient_schemes.empty())
    return true;

  for (const auto * const field_var : _sys.variableWarehouse().fieldVariables())
    if (field_var->needsGradientVectorStorage())
      return true;

  return false;
}

bool
LinearFVGradientInterface::hasRegisteredFVGradient(const unsigned int variable_number) const
{
  return _registered_gradient_schemes.count(variable_number);
}

void
LinearFVGradientInterface::initializeContainer(
    std::vector<std::unique_ptr<NumericVector<Number>>> & container) const
{
  container.clear();
  mooseAssert(_sys.currentSolution(),
              "Current solution must exist before building FV gradient storage.");
  for (unsigned int i = 0; i < _sys.mesh().dimension(); ++i)
    container.push_back(_sys.currentSolution()->zero_clone());
}

void
LinearFVGradientInterface::rebuildLinearFVGradientStorage()
{
  _raw_grad_container.clear();
  _temporary_gradient.clear();

  for (auto & limiter_container_pair : _raw_limited_grad_containers)
    limiter_container_pair.second.clear();
  for (auto & limiter_container_pair : _temporary_limited_gradient)
    limiter_container_pair.second.clear();

  if (!needsLinearFVGradientStorage())
    return;

  initializeContainer(_raw_grad_container);
  initializeContainer(_temporary_gradient);

  for (const auto limiter_type : _requested_limited_gradient_types)
  {
    if (limiter_type == Moose::FV::GradientLimiterType::None)
      continue;

    initializeContainer(_raw_limited_grad_containers[limiter_type]);
    initializeContainer(_temporary_limited_gradient[limiter_type]);
    if (!_limited_gradient_fields.count(limiter_type))
      initializeLimitedGradientField(limiter_type);
  }
}

void
LinearFVGradientInterface::initializeLimitedGradientField(
    const Moose::FV::GradientLimiterType limiter_type)
{
  _limited_gradient_fields[limiter_type] = std::make_unique<LinearFVGradientField>(
      _sys, libmesh_map_find(_raw_limited_grad_containers, limiter_type));
}

void
LinearFVGradientInterface::requestLinearFVLimitedGradients(
    const Moose::FV::GradientLimiterType limiter_type, const unsigned int variable_number)
{
  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return;

  auto * const variable =
      dynamic_cast<MooseVariableFieldBase *>(_sys.variableWarehouse().getVariable(variable_number));
  if (!variable)
    mooseError("Limited gradients were requested for variable number ",
               variable_number,
               " on system '",
               _sys.name(),
               "', but no field variable with that number exists on the system.");

  if (!variable->needsGradientVectorStorage() && !hasRegisteredFVGradient(variable_number))
    mooseError("Limited gradients were requested for variable '",
               variable->name(),
               "' on system '",
               _sys.name(),
               "', but regular gradients were not requested for that variable.");

  _requested_limited_gradient_variables[limiter_type].insert(variable_number);
  _raw_limited_grad_containers[limiter_type];

  if (!_limited_gradient_fields.count(limiter_type))
    initializeLimitedGradientField(limiter_type);

  if (_requested_limited_gradient_types.insert(limiter_type).second && !_raw_grad_container.empty())
  {
    initializeContainer(_raw_limited_grad_containers[limiter_type]);
    initializeContainer(_temporary_limited_gradient[limiter_type]);
  }
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
LinearFVGradientInterface::linearFVLimitedGradientContainer(
    const Moose::FV::GradientLimiterType limiter_type) const
{
  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return _raw_grad_container;

  const auto it = _raw_limited_grad_containers.find(limiter_type);
  if (it == _raw_limited_grad_containers.end())
    mooseError("Limited gradient container was requested but not initialized on system '",
               _sys.name(),
               "'.");

  return it->second;
}
