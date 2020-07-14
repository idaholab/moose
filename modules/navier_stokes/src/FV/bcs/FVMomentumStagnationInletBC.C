//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMomentumStagnationInletBC.h"
#include "NS.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVMomentumStagnationInletBC);

InputParameters
CNSFVMomentumStagnationInletBC::validParams()
{
  InputParameters params = CNSFVStagnationInletBC::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this BC applies to.");
  return params;
}

CNSFVMomentumStagnationInletBC::CNSFVMomentumStagnationInletBC(const InputParameters & parameters)
  : CNSFVStagnationInletBC(parameters), _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVMomentumStagnationInletBC::computeQpResidual()
{
  ADReal p_inlet, T_inlet, rho_inlet, H_inlet;
  inletConditionHelper(p_inlet, T_inlet, rho_inlet, H_inlet);
  return _eps * (rho_inlet * _velocity[_qp](_index) * (_velocity[_qp] * _normal) +
                 p_inlet * _normal(_index));
}
