//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVHLLCMomentumStagnationInletBC.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVHLLCMomentumStagnationInletBC);

InputParameters
CNSFVHLLCMomentumStagnationInletBC::validParams()
{
  InputParameters params = CNSFVHLLCStagnationInletBC::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this kernel applies to.");
  return params;
}

CNSFVHLLCMomentumStagnationInletBC::CNSFVHLLCMomentumStagnationInletBC(const InputParameters & parameters)
  : CNSFVHLLCStagnationInletBC(parameters), _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVHLLCMomentumStagnationInletBC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp] * _vel_elem[_qp](_index) +
         _normal(_index) * _pressure_elem[_qp];
}

ADReal
CNSFVHLLCMomentumStagnationInletBC::fluxBoundary()
{
  return _normal_speed_boundary * _rho_boundary * _vel_boundary(_index) +
         _normal(_index) * _p_boundary;
}

ADReal
CNSFVHLLCMomentumStagnationInletBC::hllcElem()
{
  Real f = std::sqrt(1 - MetaPhysicL::raw_value(_normal(_index)) *
                             MetaPhysicL::raw_value(_normal(_index)));
  return _normal(_index) * _SM + f * _vel_elem[_qp](_index);
}

ADReal
CNSFVHLLCMomentumStagnationInletBC::hllcBoundary()
{
  Real f = std::sqrt(1 - MetaPhysicL::raw_value(_normal(_index)) *
                             MetaPhysicL::raw_value(_normal(_index)));
  return _normal(_index) * _SM + f * _vel_boundary(_index);
}

ADReal
CNSFVHLLCMomentumStagnationInletBC::conservedVariableElem()
{
  return _rho_elem[_qp] * _vel_elem[_qp](_index);
}

ADReal
CNSFVHLLCMomentumStagnationInletBC::conservedVariableBoundary()
{
  return _rho_boundary * _vel_boundary(_index);
}
