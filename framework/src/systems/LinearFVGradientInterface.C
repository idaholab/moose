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
#include "FaceInfo.h"
#include "MathFVUtils.h"

#include "libmesh/numeric_vector.h"

using namespace libMesh;

LinearFVGradientReader
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

  auto & container = _linear_fv_gradient_container_by_method[&method];
  container.variable_numbers.insert(variable_number);

  if (container.values.empty() && _sys.currentSolution())
    initializeContainer(container.values);

  if (container.next_values.empty() && _sys.currentSolution())
    initializeContainer(container.next_values);

  return LinearFVGradientReader(_sys, container.values, method, variable_number);
}

void
LinearFVGradientInterface::computeGradients()
{
  if (_linear_fv_gradient_container_by_method.empty())
    return;

  auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
  mooseAssert(perf_graph_interface,
              "LinearFVGradientInterface requires its owning system to implement "
              "PerfGraphInterface.");
  const auto perf_id = perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
  mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
  PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

  // Keep current values unchanged until every replacement has been computed so boundary
  // conditions consistently use gradients from the previous update.
  // BCs may use cell gradients to compute the boundary face value, which is itself used to
  // compute cell gradients
  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
    computeLinearFVGradientContainer(*method_container_pair.first);

  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
    finalizeLinearFVGradientContainer(method_container_pair.second);
}

void
LinearFVGradientInterface::updateFVGradient(const LinearFVGradientReader & reader)
{
  if (&reader.system() != &_sys)
    mooseError("Requested update for a linear FV gradient field from a different system than '",
               _sys.name(),
               "'.");

  const auto method_container_pair = _linear_fv_gradient_container_by_method.find(&reader.method());
  if (method_container_pair != _linear_fv_gradient_container_by_method.end())
  {
    auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
    mooseAssert(perf_graph_interface,
                "LinearFVGradientInterface requires its owning system to implement "
                "PerfGraphInterface.");
    const auto perf_id =
        perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
    mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
    PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

    auto & container = computeLinearFVGradientContainer(reader.method());
    finalizeLinearFVGradientContainer(container);
    return;
  }

  mooseError("Requested update for an unregistered linear FV gradient field on system '",
             _sys.name(),
             "'.");
}

bool
LinearFVGradientInterface::hasLinearFVGradients() const
{
  return !_linear_fv_gradient_container_by_method.empty();
}

void
LinearFVGradientInterface::initializeContainer(GradientContainer & container) const
{
  container.clear();
  mooseAssert(_sys.currentSolution(),
              "Current solution must exist before building FV gradient storage.");
  for (unsigned int i = 0; i < _sys.mesh().dimension(); ++i)
    container.push_back(_sys.currentSolution()->zero_clone());
}

LinearFVGradientInterface::LinearFVGradientContainer &
LinearFVGradientInterface::computeLinearFVGradientContainer(const FVGradientMethod & method)
{
  auto & container = libmesh_map_find(_linear_fv_gradient_container_by_method, &method);

  if (container.values.empty())
    initializeContainer(container.values);

  if (container.next_values.empty())
    initializeContainer(container.next_values);

  mooseAssert(container.next_values.size() == container.values.size(),
              "Next and current gradient containers must have the same size.");

  method.computeGradient(_sys, container.next_values, container.variable_numbers);

  return container;
}

void
LinearFVGradientInterface::finalizeLinearFVGradientContainer(LinearFVGradientContainer & container)
{
  mooseAssert(container.next_values.size() == container.values.size(),
              "Next and current gradient containers must have the same size.");
  container.values.swap(container.next_values);
}

void
LinearFVGradientInterface::rebuildLinearFVGradientStorage()
{
  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
  {
    method_container_pair.second.values.clear();
    method_container_pair.second.next_values.clear();
  }

  if (!hasLinearFVGradients())
    return;

  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
  {
    initializeContainer(method_container_pair.second.values);
    initializeContainer(method_container_pair.second.next_values);
  }
}
