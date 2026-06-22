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

LinearFVGradientReader::LinearFVGradientReader(const SystemBase & sys,
                                               const GradientContainer & components,
                                               const FVGradientMethod & method,
                                               const unsigned int variable_number)
  : _sys(sys),
    _system_number(sys.number()),
    _components(components),
    _method(method),
    _variable_number(variable_number)
{
}

Real
LinearFVGradientReader::component(const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _components.size(), "Gradient component index out of range.");

  return (*_components[component])(elem_info.dofIndices()[_system_number][_variable_number]);
}

RealVectorValue
LinearFVGradientReader::gradient(const ElemInfo & elem_info) const
{
  RealVectorValue value;
  value.zero();

  for (const auto component_index : make_range(_sys.mesh().dimension()))
    value(component_index) = component(elem_info, component_index);

  return value;
}

RealVectorValue
LinearFVGradientReader::gradient(const FaceInfo & fi) const
{
  const auto face_type = fi.faceType(std::make_pair(_variable_number, _system_number));
  mooseAssert(face_type != FaceInfo::VarFaceNeighbors::NEITHER,
              "Gradient requested on a face where the variable is defined on neither side.");

  const bool var_defined_on_elem = (face_type == FaceInfo::VarFaceNeighbors::BOTH) ||
                                   (face_type == FaceInfo::VarFaceNeighbors::ELEM);
  const auto * const elem_one = var_defined_on_elem ? fi.elemInfo() : fi.neighborInfo();
  const auto * const elem_two = var_defined_on_elem ? fi.neighborInfo() : fi.elemInfo();

  const auto elem_one_grad = gradient(*elem_one);

  if (face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert(elem_two, "Face type indicates BOTH but neighbor information is missing.");
    const auto elem_two_grad = gradient(*elem_two);
    return Moose::FV::linearInterpolation(elem_one_grad, elem_two_grad, fi, var_defined_on_elem);
  }
  else
    return elem_one_grad;
}

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

  method.setupDependencies(_sys, variable_number);

  auto & container = _linear_fv_gradient_container_by_method[&method];
  container.variable_numbers.insert(variable_number);

  if (container.values.empty() && _sys.currentSolution())
    initializeContainer(container.values);

  if (_linear_fv_gradient_output_scratch.empty() && _sys.currentSolution())
    initializeContainer(_linear_fv_gradient_output_scratch);

  if (_linear_fv_gradient_method_scratch.empty() && _sys.currentSolution())
    initializeContainer(_linear_fv_gradient_method_scratch);

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

  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
    updateLinearFVGradientContainer(*method_container_pair.first, method_container_pair.second);
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

    updateLinearFVGradientContainer(reader.method(), method_container_pair->second);
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

void
LinearFVGradientInterface::updateLinearFVGradientContainer(const FVGradientMethod & method,
                                                           LinearFVGradientContainer & container)
{
  if (container.values.empty())
    initializeContainer(container.values);

  if (_linear_fv_gradient_output_scratch.empty())
    initializeContainer(_linear_fv_gradient_output_scratch);

  if (_linear_fv_gradient_method_scratch.empty())
    initializeContainer(_linear_fv_gradient_method_scratch);

  mooseAssert(_linear_fv_gradient_output_scratch.size() == container.values.size(),
              "Output scratch and value gradient containers must have the same size.");
  mooseAssert(_linear_fv_gradient_method_scratch.size() == container.values.size(),
              "Method scratch and value gradient containers must have the same size.");

  method.computeGradient(_sys,
                         _linear_fv_gradient_output_scratch,
                         _linear_fv_gradient_method_scratch,
                         container.variable_numbers);

  container.values.swap(_linear_fv_gradient_output_scratch);
}

void
LinearFVGradientInterface::rebuildLinearFVGradientStorage()
{
  _linear_fv_gradient_output_scratch.clear();
  _linear_fv_gradient_method_scratch.clear();

  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
    method_container_pair.second.values.clear();

  if (!hasLinearFVGradients())
    return;

  initializeContainer(_linear_fv_gradient_output_scratch);
  initializeContainer(_linear_fv_gradient_method_scratch);

  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
    initializeContainer(method_container_pair.second.values);
}
