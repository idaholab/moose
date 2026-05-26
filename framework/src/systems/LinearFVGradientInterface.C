//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGradientInterface.h"

#include "ComputeLinearFVLimitedGradientThread.h"
#include "FEProblemBase.h"
#include "FVGradientMethod.h"
#include "PerfGraphInterface.h"
#include "PerfGuard.h"
#include "SystemBase.h"
#include "MooseVariableFieldBase.h"
#include "MooseError.h"
#include "ElemInfo.h"

#include "libmesh/numeric_vector.h"

using namespace libMesh;

LinearFVGradientField::LinearFVGradientField(const SystemBase & sys,
                                             const GradientContainer & components,
                                             const Moose::FV::GradientLimiterType limiter_type)
  : _sys(sys), _components(components), _limiter_type(limiter_type)
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

LinearFVGradientField &
LinearFVGradientInterface::registerFVGradient(const unsigned int variable_number,
                                              const FVGradientMethod & method)
{
  auto * const variable =
      dynamic_cast<MooseVariableFieldBase *>(_sys.variableWarehouse().getVariable(variable_number));
  if (!variable)
    mooseError("Linear FV gradients were requested for variable number ",
               variable_number,
               " on system '",
               _sys.name(),
               "', but no field variable with that number exists on the system.");

  auto & storage = methodGradientStorage(method);
  storage.variable_numbers.insert(variable_number);

  if (storage.values.empty() && _sys.currentSolution())
    initializeMethodGradientStorage(storage);

  return *storage.field;
}

void
LinearFVGradientInterface::computeGradients()
{
  if (_registered_gradient_method_fields.empty())
    return;

  auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
  mooseAssert(perf_graph_interface,
              "LinearFVGradientInterface requires its owning system to implement "
              "PerfGraphInterface.");
  const auto perf_id = perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
  mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
  PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

  for (auto & method_field_pair : _registered_gradient_method_fields)
    updateMethodGradientStorage(*method_field_pair.second);
}

void
LinearFVGradientInterface::updateFVGradient(const LinearFVGradientField & field)
{
  if (&field.system() != &_sys)
    mooseError("Requested update for a linear FV gradient field from a different system than '",
               _sys.name(),
               "'.");

  for (auto & method_field_pair : _registered_gradient_method_fields)
    if (method_field_pair.second->field.get() == &field)
    {
      auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
      mooseAssert(perf_graph_interface,
                  "LinearFVGradientInterface requires its owning system to implement "
                  "PerfGraphInterface.");
      const auto perf_id =
          perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
      mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
      PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

      updateMethodGradientStorage(*method_field_pair.second);
      return;
    }

  mooseError("Requested update for an unregistered linear FV gradient field on system '",
             _sys.name(),
             "'.");
}

bool
LinearFVGradientInterface::needsLinearFVGradientStorage() const
{
  return !_registered_gradient_method_fields.empty();
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

LinearFVGradientInterface::LinearFVGradientFieldStorage &
LinearFVGradientInterface::methodGradientStorage(const FVGradientMethod & method)
{
  auto it = _registered_gradient_method_fields.find(&method);
  if (it != _registered_gradient_method_fields.end())
    return *it->second;

  auto storage = std::make_unique<LinearFVGradientFieldStorage>(method);
  storage->field =
      std::make_unique<LinearFVGradientField>(_sys, storage->values, method.limiterType());

  auto & storage_ref = *storage;
  _registered_gradient_method_fields.emplace(&method, std::move(storage));
  return storage_ref;
}

void
LinearFVGradientInterface::initializeMethodGradientStorage(LinearFVGradientFieldStorage & storage)
{
  initializeContainer(storage.values);
  initializeContainer(storage.output_scratch);

  if (storage.method.limiterType() != Moose::FV::GradientLimiterType::None)
    initializeContainer(storage.base_scratch);
}

void
LinearFVGradientInterface::updateMethodGradientStorage(LinearFVGradientFieldStorage & storage)
{
  if (storage.values.empty())
    initializeMethodGradientStorage(storage);

  if (storage.method.limiterType() == Moose::FV::GradientLimiterType::None)
  {
    auto & output_scratch = storage.output_scratch;
    mooseAssert(output_scratch.size() == storage.values.size(),
                "Output scratch and value method gradient containers must have the same size.");
    for (auto & vec : output_scratch)
      vec->zero();

    storage.method.computeGradient(_sys, output_scratch, storage.variable_numbers);

    for (auto & vec : output_scratch)
      vec->close();

    storage.values.swap(output_scratch);
    return;
  }

  auto & base_scratch = storage.base_scratch;
  mooseAssert(base_scratch.size() == storage.values.size(),
              "Base scratch and value method gradient containers must have the same size.");
  for (auto & vec : base_scratch)
    vec->zero();

  storage.method.computeGradient(_sys, base_scratch, storage.variable_numbers);

  for (auto & vec : base_scratch)
    vec->close();

  auto & fe_problem = _sys.feProblem();
  using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
  ElemInfoRange elem_info_range(fe_problem.mesh().ownedElemInfoBegin(),
                                fe_problem.mesh().ownedElemInfoEnd());

  auto & output_scratch = storage.output_scratch;
  mooseAssert(output_scratch.size() == storage.values.size(),
              "Output scratch and value method gradient containers must have the same size.");
  for (auto & vec : output_scratch)
    vec->zero();

  PARALLEL_TRY
  {
    ComputeLinearFVLimitedGradientThread limited_gradient_thread(fe_problem,
                                                                 _sys,
                                                                 base_scratch,
                                                                 output_scratch,
                                                                 storage.method.limiterType(),
                                                                 storage.variable_numbers);
    Threads::parallel_reduce(elem_info_range, limited_gradient_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (auto & vec : output_scratch)
    vec->close();

  storage.values.swap(output_scratch);
}

void
LinearFVGradientInterface::rebuildLinearFVGradientStorage()
{
  for (auto & method_field_pair : _registered_gradient_method_fields)
  {
    method_field_pair.second->base_scratch.clear();
    method_field_pair.second->values.clear();
    method_field_pair.second->output_scratch.clear();
  }

  if (!needsLinearFVGradientStorage())
    return;

  for (auto & method_field_pair : _registered_gradient_method_fields)
    initializeMethodGradientStorage(*method_field_pair.second);
}
