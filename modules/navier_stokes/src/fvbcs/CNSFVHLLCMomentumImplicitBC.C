//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCMomentumImplicitBC.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVHLLCMomentumImplicitBC);

InputParameters
CNSFVHLLCMomentumImplicitBC::validParams()
{
  InputParameters params = CNSFVHLLCImplicitBC::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this kernel applies to.");
  params.addClassDescription("Implements an implicit advective boundary flux for the momentum "
                             "equation for an HLLC discretization");
  return params;
}

CNSFVHLLCMomentumImplicitBC::CNSFVHLLCMomentumImplicitBC(const InputParameters & parameters)
  : CNSFVHLLCImplicitBC(parameters), _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVHLLCMomentumImplicitBC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp] * _vel_elem[_qp](_index) +
         _normal(_index) * _pressure_elem[_qp];
}

ADReal
CNSFVHLLCMomentumImplicitBC::hllcElem()
{
  auto vel_nonnormal = _vel_elem[_qp] - _normal_speed_elem * _normal;
  return _normal(_index) * _SM + vel_nonnormal(_index);

  // For some reason, the below expression doesn't give as good results as the
  // above one.
  // return _normal(_index) * (_SM - _normal_speed_elem) + _vel_elem[_qp](_index);
}

ADReal
CNSFVHLLCMomentumImplicitBC::conservedVariableElem()
{
  return _rho_elem[_qp] * _vel_elem[_qp](_index);
}
