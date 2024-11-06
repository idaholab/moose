//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapPerfectConductance.h"

registerMooseObject("HeatTransferApp", GapPerfectConductance);

InputParameters
GapPerfectConductance::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Enforces equal temperatures across the gap.");
  params.addRequiredCoupledVar("gap_distance", "Distance across the gap.");
  params.addRequiredCoupledVar("gap_temp", "Temperature on the other side of the gap.");
  params.addParam<Real>("penalty", 1e3, "Penalty value to be applied to the constraint.");
  return params;
}

GapPerfectConductance::GapPerfectConductance(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _gap_distance(coupledValue("gap_distance")),
    _gap_temp(coupledValue("gap_temp")),
    _penalty(getParam<Real>("penalty"))
{
}

Real
GapPerfectConductance::computeQpResidual()
{
  return _penalty * (_u[_qp] - _gap_temp[_qp]);
}

Real
GapPerfectConductance::computeQpJacobian()
{
  return _penalty;
}
