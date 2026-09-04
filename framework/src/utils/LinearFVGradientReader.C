//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGradientReader.h"
#include "FEProblemBase.h"
#include "FVGradientMethod.h"
#include "SystemBase.h"
#include "MooseVariableFieldBase.h"
#include "MooseError.h"
#include "ElemInfo.h"
#include "FaceInfo.h"
#include "MathFVUtils.h"
#include "Conversion.h"

#include "libmesh/numeric_vector.h"

using namespace libMesh;

LinearFVGradientReader::LinearFVGradientReader(const SystemBase & sys,
                                               const GradientStateContainer & state_values,
                                               const FVGradientMethod & method,
                                               const unsigned int variable_number)
  : _sys(sys),
    _system_number(sys.number()),
    _state_values(state_values),
    _method(method),
    _variable_number(variable_number)
{
  mooseAssert(!_state_values.empty(), "Gradient state storage must contain a current state.");
}

Real
LinearFVGradientReader::component(const ElemInfo & elem_info, const unsigned int index) const
{
  return component(elem_info, index, Moose::currentState());
}

const LinearFVGradientReader::GradientContainer &
LinearFVGradientReader::components() const
{
  return components(Moose::currentState());
}

const LinearFVGradientReader::GradientContainer &
LinearFVGradientReader::components(const Moose::StateArg & state) const
{
  return stateComponents(state);
}

const LinearFVGradientReader::GradientContainer &
LinearFVGradientReader::stateComponents(const Moose::StateArg & state) const
{
  mooseAssert(!_state_values.empty(), "Gradient state storage must contain a current state.");

  if (state.state > 0 && state.iteration_type != Moose::SolutionIterationType::Time)
    mooseError("Linear FV gradient state ",
               state.state,
               " with iteration type '",
               Moose::stringify(state.iteration_type),
               "' was requested for variable number ",
               _variable_number,
               " on system '",
               _sys.name(),
               "' using gradient method '",
               _method.name(),
               "'. Only time iteration states are supported for non-current gradients.");

  if (state.state >= _state_values.size())
    mooseError("Linear FV gradient state ",
               state.state,
               " with iteration type '",
               Moose::stringify(state.iteration_type),
               "' was requested for variable number ",
               _variable_number,
               " on system '",
               _sys.name(),
               "' using gradient method '",
               _method.name(),
               "', but the maximum allocated state is ",
               _state_values.size() - 1,
               ".");

  return _state_values[state.state];
}

Real
LinearFVGradientReader::component(const ElemInfo & elem_info,
                                  const unsigned int index,
                                  const Moose::StateArg & state) const
{
  const auto & components = stateComponents(state);
  mooseAssert(index < components.size(), "Gradient component index out of range.");
  mooseAssert(components[index], "Gradient component vector must be initialized.");

  return (*components[index])(elem_info.dofIndices()[_system_number][_variable_number]);
}

RealVectorValue
LinearFVGradientReader::gradient(const ElemInfo & elem_info) const
{
  return gradient(elem_info, Moose::currentState());
}

RealVectorValue
LinearFVGradientReader::gradient(const ElemInfo & elem_info, const Moose::StateArg & state) const
{
  RealVectorValue value;

  for (const auto component_index : make_range(elem_info.elem()->dim()))
    value(component_index) = component(elem_info, component_index, state);

  return value;
}

RealVectorValue
LinearFVGradientReader::gradient(const FaceInfo & fi) const
{
  return gradient(fi, Moose::currentState());
}

RealVectorValue
LinearFVGradientReader::gradient(const FaceInfo & fi, const Moose::StateArg & state) const
{
  const auto face_type = fi.faceType(std::make_pair(_variable_number, _system_number));
  mooseAssert(face_type != FaceInfo::VarFaceNeighbors::NEITHER,
              "Gradient requested on a face where the variable is defined on neither side.");

  const bool var_defined_on_elem = (face_type == FaceInfo::VarFaceNeighbors::BOTH) ||
                                   (face_type == FaceInfo::VarFaceNeighbors::ELEM);
  const auto * const elem_one = var_defined_on_elem ? fi.elemInfo() : fi.neighborInfo();
  const auto * const elem_two = var_defined_on_elem ? fi.neighborInfo() : fi.elemInfo();

  const auto elem_one_grad = gradient(*elem_one, state);

  if (face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert(elem_two, "Face type indicates BOTH but neighbor information is missing.");
    const auto elem_two_grad = gradient(*elem_two, state);
    return Moose::FV::linearInterpolation(elem_one_grad, elem_two_grad, fi, var_defined_on_elem);
  }
  else
    return elem_one_grad;
}
