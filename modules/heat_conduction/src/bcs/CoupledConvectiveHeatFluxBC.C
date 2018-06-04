//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledConvectiveHeatFluxBC.h"

registerMooseObject("HeatConductionApp", CoupledConvectiveHeatFluxBC);

template <>
InputParameters
validParams<CoupledConvectiveHeatFluxBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by auxiliary variables.");
  params.addCoupledVar("alpha", 1., "Volume fraction of components");
  params.addRequiredCoupledVar("T_infinity", "Field holding far-field temperature");
  params.addRequiredCoupledVar("htc", "Heat transfer coefficient");

  return params;
}

CoupledConvectiveHeatFluxBC::CoupledConvectiveHeatFluxBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _n_components(coupledComponents("T_infinity"))
{
  if (coupledComponents("alpha") != _n_components)
    paramError(
        "alpha",
        "The number of coupled components does not match the number of `T_infinity` components.");
  if (coupledComponents("htc") != _n_components)
    paramError(
        "htc",
        "The number of coupled components does not match the number of `T_infinity` components.");

  _htc.resize(_n_components);
  _T_infinity.resize(_n_components);
  _alpha.resize(_n_components);
  for (std::size_t c = 0; c < _n_components; c++)
  {
    _htc[c] = &coupledValue("htc", c);
    _T_infinity[c] = &coupledValue("T_infinity", c);
    _alpha[c] = &coupledValue("alpha", c);
  }
}

Real
CoupledConvectiveHeatFluxBC::computeQpResidual()
{
  Real q = 0;
  for (std::size_t c = 0; c < _n_components; c++)
    q += (*_alpha[c])[_qp] * (*_htc[c])[_qp] * (_u[_qp] - (*_T_infinity[c])[_qp]);
  return _test[_i][_qp] * q;
}

Real
CoupledConvectiveHeatFluxBC::computeQpJacobian()
{
  Real dq = 0;
  for (std::size_t c = 0; c < _n_components; c++)
    dq += (*_alpha[c])[_qp] * (*_htc[c])[_qp] * _phi[_j][_qp];
  return _test[_i][_qp] * dq;
}
