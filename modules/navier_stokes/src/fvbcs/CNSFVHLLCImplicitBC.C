//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCImplicitBC.h"

InputParameters
CNSFVHLLCImplicitBC::validParams()
{
  InputParameters params = CNSFVHLLCBC::validParams();
  return params;
}

CNSFVHLLCImplicitBC::CNSFVHLLCImplicitBC(const InputParameters & parameters)
  : CNSFVHLLCBC(parameters)
{
}

void
CNSFVHLLCImplicitBC::preComputeWaveSpeed()
{
  _normal_speed_boundary = _normal_speed_elem;
  _rho_boundary = _rho_elem[_qp];
  _vel_boundary = _vel_elem[_qp];
  _specific_internal_energy_boundary = _specific_internal_energy_elem[_qp];
}

ADReal
CNSFVHLLCImplicitBC::fluxBoundary()
{
  return fluxElem();
}

ADReal
CNSFVHLLCImplicitBC::hllcBoundary()
{
  return hllcElem();
}

ADReal
CNSFVHLLCImplicitBC::conservedVariableBoundary()
{
  return conservedVariableElem();
}
