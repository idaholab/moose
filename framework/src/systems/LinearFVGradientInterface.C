//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGradientInterface.h"

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
                                             const FVGradientMethod & method)
  : _sys(sys), _components(components), _method(method)
{
}

Moose::FV::GradientLimiterType
LinearFVGradientField::limiterType() const
{
  return _method.limiterType();
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
  storage->field = std::make_unique<LinearFVGradientField>(_sys, storage->values, method);

  auto & storage_ref = *storage;
  _registered_gradient_method_fields.emplace(&method, std::move(storage));
  return storage_ref;
}

void
LinearFVGradientInterface::initializeMethodGradientStorage(LinearFVGradientFieldStorage & storage)
{
  initializeContainer(storage.values);
  initializeContainer(storage.output_scratch);
  initializeContainer(storage.method_scratch);
}

void
LinearFVGradientInterface::updateMethodGradientStorage(LinearFVGradientFieldStorage & storage)
{
  if (storage.values.empty())
    initializeMethodGradientStorage(storage);

  auto & output_scratch = storage.output_scratch;
  mooseAssert(output_scratch.size() == storage.values.size(),
              "Output scratch and value method gradient containers must have the same size.");
  mooseAssert(storage.method_scratch.size() == storage.values.size(),
              "Method scratch and value gradient containers must have the same size.");

  storage.method.computeGradient(
      _sys, output_scratch, storage.method_scratch, storage.variable_numbers);

  storage.values.swap(output_scratch);
}

void
LinearFVGradientInterface::rebuildLinearFVGradientStorage()
{
  for (auto & method_field_pair : _registered_gradient_method_fields)
  {
    method_field_pair.second->method_scratch.clear();
    method_field_pair.second->values.clear();
    method_field_pair.second->output_scratch.clear();
  }

  if (!needsLinearFVGradientStorage())
    return;

  for (auto & method_field_pair : _registered_gradient_method_fields)
    initializeMethodGradientStorage(*method_field_pair.second);
}
