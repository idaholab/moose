//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVElementalAdvection.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

registerMooseObject("MooseTestApp", FVElementalAdvection);

InputParameters
FVElementalAdvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  return params;
}

FVElementalAdvection::FVElementalAdvection(const InputParameters & params)
  : FVElementalKernel(params), _velocity(getParam<RealVectorValue>("velocity"))
{
}

ADReal
FVElementalAdvection::computeQpResidual()
{
  return _velocity * _var.adGradSln(_current_elem);
}

#endif
