//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVScalarOutflowBC.h"

registerMooseObject("MooseApp", FVScalarOutflowBC);

InputParameters
FVScalarOutflowBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Velocity scalar advection boundary conditions for finite volume method.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  return params;
}

FVScalarOutflowBC::FVScalarOutflowBC(const InputParameters & params)
  : FVFluxBC(params),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v")
             ? &(getFunctor<ADReal>("v"))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? &(getFunctor<ADReal>("w"))
               : nullptr)
{
}

ADReal
FVScalarOutflowBC::computeQpResidual()
{

  //const Elem & _current_elem = _face_info->elem(); // keeping just in case we want to switch back to elemental BCs later on

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(singleSidedFaceArg()));
  if (_v_var)
      velocity(1) = (*_v_var)(singleSidedFaceArg());
  if (_w_var)
      velocity(2) = (*_w_var)(singleSidedFaceArg());

#ifdef MOOSE_GLOBAL_AD_INDEXING
  // This will either be second or first order accurate depending on whether the user has asked
  // for a two term expansion in their input file
  return _normal * velocity * _var.getBoundaryFaceValue(*_face_info);
#else
  // this simply returns a cell centroid value which will only result in first order accuracy
  return _normal * velocity * uOnUSub();
#endif
}