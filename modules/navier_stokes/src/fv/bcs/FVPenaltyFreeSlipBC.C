//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPenaltyFreeSlipBC.h"

registerMooseObject("NavierStokesApp", FVPenaltyFreeSlipBC);

InputParameters
FVPenaltyFreeSlipBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", 0, "The velocity in the y direction.");
  params.addCoupledVar("w", 0, "The velocity in the z direction.");
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this BC applies to.");
  params.addParam<Real>("penalty", 1e6, "The penalty factor");
  return params;
}

FVPenaltyFreeSlipBC::FVPenaltyFreeSlipBC(const InputParameters & params)
  : FVFluxBC(params),
    _u(adCoupledValue("u")),
    _v(adCoupledValue("v")),
    _w(adCoupledValue("w")),
    _comp(getParam<MooseEnum>("momentum_component")),
    _p(getParam<Real>("penalty"))

{
}

ADReal
FVPenaltyFreeSlipBC::computeQpResidual()
{
  return _p * _normal(_comp) * (_normal * ADRealVectorValue(_u[_qp], _v[_qp], _w[_qp]));
}
