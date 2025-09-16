//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCoupledConvectiveHeatFluxBC.h"

registerMooseObject("HeatTransferApp", KokkosCoupledConvectiveHeatFluxBC);

InputParameters
KokkosCoupledConvectiveHeatFluxBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by auxiliary variables.");
  params.addCoupledVar("scale_factor", 1., "Scale factor to multiply the heat flux with");
  params.addCoupledVar("alpha", 1., "Volume fraction of components");
  params.addRequiredCoupledVar("T_infinity", "Field holding far-field temperature");
  params.addRequiredCoupledVar("htc", "Heat transfer coefficient");

  return params;
}

KokkosCoupledConvectiveHeatFluxBC::KokkosCoupledConvectiveHeatFluxBC(
    const InputParameters & parameters)
  : IntegratedBC(parameters),
    _n_components(coupledComponents("T_infinity")),
    _T_infinity(kokkosCoupledValues("T_infinity")),
    _htc(kokkosCoupledValues("htc")),
    _alpha(kokkosCoupledValues("alpha")),
    _scale_factor(kokkosCoupledValue("scale_factor"))
{
  if (coupledComponents("alpha") != _n_components)
    paramError(
        "alpha",
        "The number of coupled components does not match the number of `T_infinity` components.");
  if (coupledComponents("htc") != _n_components)
    paramError(
        "htc",
        "The number of coupled components does not match the number of `T_infinity` components.");
}
