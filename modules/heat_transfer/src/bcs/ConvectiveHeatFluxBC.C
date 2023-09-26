//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectiveHeatFluxBC.h"

registerMooseObject("HeatConductionApp", ConvectiveHeatFluxBC);

InputParameters
ConvectiveHeatFluxBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by material properties.");
  params.addRequiredParam<MaterialPropertyName>("T_infinity",
                                                "Material property for far-field temperature");
  params.addRequiredParam<MaterialPropertyName>("heat_transfer_coefficient",
                                                "Material property for heat transfer coefficient");
  params.addParam<MaterialPropertyName>(
      "heat_transfer_coefficient_dT",
      "0",
      "Material property for derivative of heat transfer coefficient with respect to temperature");

  return params;
}

ConvectiveHeatFluxBC::ConvectiveHeatFluxBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_infinity(getMaterialProperty<Real>("T_infinity")),
    _htc(getMaterialProperty<Real>("heat_transfer_coefficient")),
    _htc_dT(getMaterialProperty<Real>("heat_transfer_coefficient_dT"))
{
}

Real
ConvectiveHeatFluxBC::computeQpResidual()
{
  return -_test[_i][_qp] * _htc[_qp] * (_T_infinity[_qp] - _u[_qp]);
}

Real
ConvectiveHeatFluxBC::computeQpJacobian()
{
  return -_test[_i][_qp] * _phi[_j][_qp] *
         (-_htc[_qp] + _htc_dT[_qp] * (_T_infinity[_qp] - _u[_qp]));
}
