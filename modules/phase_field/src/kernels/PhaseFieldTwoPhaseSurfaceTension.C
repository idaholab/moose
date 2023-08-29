/****************************************************************************/
/*                        DO NOT MODIFY THIS HEADER                         */
/*                                                                          */
/* MALAMUTE: MOOSE Application Library for Advanced Manufacturing UTilitiEs */
/*                                                                          */
/*           Copyright 2021 - 2023, Battelle Energy Alliance, LLC           */
/*                           ALL RIGHTS RESERVED                            */
/****************************************************************************/

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

PhaseFieldTwoPhaseSurfaceTension::PhaseFieldTwoPhaseSurfaceTension(const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _auxpf(adCoupledValue("auxpf")),
    _grad_pf(adCoupledGradient("pf")),
    _coeff(getParam<Real>("coeff"))
{
}

ADRealVectorValue
PhaseFieldTwoPhaseSurfaceTension::precomputeQpResidual()
{
  return -_coeff*_auxpf[_qp]*_grad_pf[_qp];
}
 