//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConvectiveHeatFluxBC.h"

registerKokkosResidualObject("HeatTransferApp", KokkosConvectiveHeatFluxBC);

InputParameters
KokkosConvectiveHeatFluxBC::validParams()
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

KokkosConvectiveHeatFluxBC::KokkosConvectiveHeatFluxBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_infinity(getKokkosMaterialProperty<Real>("T_infinity")),
    _htc(getKokkosMaterialProperty<Real>("heat_transfer_coefficient")),
    _htc_dT(getKokkosMaterialProperty<Real>("heat_transfer_coefficient_dT"))
{
}
