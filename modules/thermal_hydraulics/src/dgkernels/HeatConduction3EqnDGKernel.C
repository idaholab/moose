//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConduction3EqnDGKernel.h"

registerMooseObject("ThermalHydraulicsApp", HeatConduction3EqnDGKernel);

InputParameters
HeatConduction3EqnDGKernel::validParams()
{
  InputParameters params = ADDGKernel::validParams();

  params.addRequiredParam<MaterialPropertyName>("k", "Thermal conductivity");
  params.addRequiredParam<MaterialPropertyName>("T", "Temperature");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("direction", "Direction of the flow channel");

  params.addClassDescription("Adds heat conduction for the single-phase flow model.");

  return params;
}

HeatConduction3EqnDGKernel::HeatConduction3EqnDGKernel(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _k_elem(getADMaterialProperty<Real>("k")),
    _k_neig(getNeighborADMaterialProperty<Real>("k")),
    _T_elem(getADMaterialProperty<Real>("T")),
    _T_neig(getNeighborADMaterialProperty<Real>("T")),
    _A(adCoupledValue("A")),
    _dir(getMaterialProperty<RealVectorValue>("direction"))
{
}

ADReal
HeatConduction3EqnDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  const ADReal k_avg = 0.5 * (_k_elem[_qp] + _k_neig[_qp]);

  const Point x_elem = _current_elem->vertex_average();
  const Point x_neig = _neighbor_elem->vertex_average();
  const Real dx = (x_neig - x_elem) * _dir[_qp];
  const ADReal dTdx = (_T_neig[_qp] - _T_elem[_qp]) / dx;

  const ADReal flux = k_avg * dTdx * _A[_qp];

  return type == Moose::Element ? -flux * _test[_i][_qp] : flux * _test_neighbor[_i][_qp];
}
