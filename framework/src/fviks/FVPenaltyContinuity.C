//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPenaltyContinuity.h"
#include "MathFVUtils.h"

registerMooseObject("MooseApp", FVPenaltyContinuity);

InputParameters
FVPenaltyContinuity::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.set<unsigned short>("ghost_layers") = 2;
  params.addRequiredParam<Real>("penalty", "The penalty");
  return params;
}

FVPenaltyContinuity::FVPenaltyContinuity(const InputParameters & params)
  : FVInterfaceKernel(params), _penalty(getParam<Real>("penalty"))
{
}

ADReal
FVPenaltyContinuity::computeQpResidual()
{
  return _penalty *
         (var1().getBoundaryFaceValue(*_face_info) - var2().getBoundaryFaceValue(*_face_info));
}
