//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCMassImplicitBC.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVHLLCMassImplicitBC);

InputParameters
CNSFVHLLCMassImplicitBC::validParams()
{
  InputParameters params = CNSFVHLLCImplicitBC::validParams();
  return params;
}

CNSFVHLLCMassImplicitBC::CNSFVHLLCMassImplicitBC(const InputParameters & parameters)
  : CNSFVHLLCImplicitBC(parameters)
{
}

ADReal
CNSFVHLLCMassImplicitBC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp];
}

ADReal
CNSFVHLLCMassImplicitBC::hllcElem()
{
  return 1;
}

ADReal
CNSFVHLLCMassImplicitBC::conservedVariableElem()
{
  return _rho_elem[_qp];
}
