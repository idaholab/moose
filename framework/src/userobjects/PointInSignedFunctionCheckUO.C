//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInSignedFunctionCheckUO.h"
#include "Function.h"
#include "SurfaceSide.h"

registerMooseObject("MooseApp", PointInSignedFunctionCheckUO);

InputParameters
PointInSignedFunctionCheckUO::validParams()
{
  InputParameters params = ThreadedGeneralUserObject::validParams();
  params.addClassDescription(
      "Classifies a point as inside, on, or outside a surface defined by the "
      "zero level set of a signed function.");
  params.addRequiredParam<FunctionName>(
      "function",
      "Signed level-set function; negative inside by default (see inside_is_negative).");
  params.addParam<Real>("tolerance",
                        libMesh::TOLERANCE,
                        "Half-width, in function value space, of the on-surface band around the "
                        "zero level set. Points within this band are labeled as on the surface.");
  params.addParam<bool>(
      "inside_is_negative",
      true,
      "Whether negative function values denote the interior. True matches the signed-distance "
      "convention; set false for a level set that is positive inside.");
  return params;
}

PointInSignedFunctionCheckUO::PointInSignedFunctionCheckUO(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _func(getFunction("function")),
    _tolerance(getParam<Real>("tolerance")),
    _inside_is_negative(getParam<bool>("inside_is_negative"))
{
  if (_tolerance <= 0.0)
    paramError("tolerance", "must be greater than zero.");
}

SurfaceGeometry::SurfaceSide
PointInSignedFunctionCheckUO::sideness(const Point & p) const
{
  return SurfaceGeometry::signedValueSideness(_func.value(_t, p), _tolerance, _inside_is_negative);
}
