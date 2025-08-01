//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneD3EqnEnergyHeatSource.h"

registerMooseObject("ThermalHydraulicsApp", ADOneD3EqnEnergyHeatSource);

InputParameters
ADOneD3EqnEnergyHeatSource::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredParam<MooseFunctorName>("q", "Volumetric heat source");
  params.addRequiredCoupledVar("A", "Cross sectional area");
  params.addClassDescription("Computes a volumetric heat source for 1-phase flow channel");
  return params;
}

ADOneD3EqnEnergyHeatSource::ADOneD3EqnEnergyHeatSource(const InputParameters & parameters)
  : ADKernel(parameters), _q(getFunctor<ADReal>("q")), _A(coupledValue("A"))
{
}

ADReal
ADOneD3EqnEnergyHeatSource::computeQpResidual()
{
  const Moose::ElemQpArg qp_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  return -_q(qp_arg, Moose::currentState()) * _A[_qp] * _test[_i][_qp];
}
