//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMassAdvection.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

registerMooseObject("NavierStokesApp", INSFVMassAdvection);

InputParameters
INSFVMassAdvection::validParams()
{
  return INSFVMomentumAdvection::validParams();
}

INSFVMassAdvection::INSFVMassAdvection(const InputParameters & params)
  : INSFVMomentumAdvection(params)
{
}

ADReal
INSFVMassAdvection::computeQpResidual()
{
  ADRealVectorValue v;

  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  return _normal * v * _rho;
}

#endif
