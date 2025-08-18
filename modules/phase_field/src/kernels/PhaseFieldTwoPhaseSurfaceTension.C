//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldTwoPhaseSurfaceTension.h"

registerMooseObject("PhaseFieldApp", PhaseFieldTwoPhaseSurfaceTension);

InputParameters
PhaseFieldTwoPhaseSurfaceTension::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addRequiredCoupledVar("auxpf", "Auxillary variable in phase field formulation");
  params.addRequiredCoupledVar("pf", "phase field variable");
  params.addRequiredParam<Real>("coeff", "prefactor relating chemical potential and psi");
  return params;
}

PhaseFieldTwoPhaseSurfaceTension::PhaseFieldTwoPhaseSurfaceTension(
    const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _auxpf(adCoupledValue("auxpf")),
    _grad_pf(adCoupledGradient("pf")),
    _coeff(getParam<Real>("coeff"))
{
}

ADRealVectorValue
PhaseFieldTwoPhaseSurfaceTension::precomputeQpResidual()
{
  return -_coeff * _auxpf[_qp] * _grad_pf[_qp];
}
