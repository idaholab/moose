//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFluidEnergyStagnationInletBC.h"
#include "NS.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVFluidEnergyStagnationInletBC);

InputParameters
CNSFVFluidEnergyStagnationInletBC::validParams()
{
  InputParameters params = CNSFVStagnationInletBC::validParams();
  return params;
}

CNSFVFluidEnergyStagnationInletBC::CNSFVFluidEnergyStagnationInletBC(const InputParameters & parameters)
  : CNSFVStagnationInletBC(parameters)
{
}

ADReal
CNSFVFluidEnergyStagnationInletBC::computeQpResidual()
{
  ADReal p_inlet, T_inlet, rho_inlet, H_inlet;
  inletConditionHelper(p_inlet, T_inlet, rho_inlet, H_inlet);
  return _eps * H_inlet * _velocity[_qp] * _normal;
}
