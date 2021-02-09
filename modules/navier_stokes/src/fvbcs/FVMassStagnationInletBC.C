//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMassStagnationInletBC.h"
#include "NS.h"

registerADMooseObject("NavierStokesApp", CNSFVMassStagnationInletBC);

InputParameters
CNSFVMassStagnationInletBC::validParams()
{
  InputParameters params = CNSFVStagnationInletBC::validParams();
  return params;
}

CNSFVMassStagnationInletBC::CNSFVMassStagnationInletBC(const InputParameters & parameters)
  : CNSFVStagnationInletBC(parameters)
{
}

ADReal
CNSFVMassStagnationInletBC::computeQpResidual()
{
  ADReal p_inlet, T_inlet, rho_inlet, H_inlet;
  inletConditionHelper(p_inlet, T_inlet, rho_inlet, H_inlet);
  return _eps * rho_inlet * _velocity[_qp] * _normal;
}
