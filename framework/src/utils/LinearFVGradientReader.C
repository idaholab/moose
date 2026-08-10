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

  for (const auto component_index : make_range(elem_info.elem()->dim()))
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
