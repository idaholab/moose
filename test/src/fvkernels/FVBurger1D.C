//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVBurger1D.h"

registerMooseObject("MooseTestApp", FVBurger1D);

InputParameters
FVBurger1D::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  return params;
}

FVBurger1D::FVBurger1D(const InputParameters & params) : FVFluxKernel(params) {}

ADReal
FVBurger1D::computeQpResidual()
{
  mooseAssert(_face_info->elem().dim() == 1, "FVBurger1D works only in 1D");
  ADReal r = 0;
  ADReal u_av = 0.5 * (_u_elem[_qp] + _u_neighbor[_qp]);
  if (u_av * _normal(0) > 0)
    r = 0.5 * _u_elem[_qp] * _u_elem[_qp] * _normal(0);
  else
    r = 0.5 * _u_neighbor[_qp] * _u_neighbor[_qp] * _normal(0);
  return r;
}
