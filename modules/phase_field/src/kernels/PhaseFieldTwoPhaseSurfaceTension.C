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
  params.addRequiredCoupledVar("psi", "Auxillary variable in phase field formulation");
  params.addRequiredCoupledVar("phi", "phase field variable");
  params.addRequiredParam<Real>("coeff", "prefactor relating chemical potential and psi");
  return params;
}

PhaseFieldTwoPhaseSurfaceTension::PhaseFieldTwoPhaseSurfaceTension(const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _psi(adCoupledValue("psi")),
    _grad_phi(adCoupledGradient("phi")),
    _coeff(getParam<Real>("coeff"))
{
}

ADRealVectorValue
PhaseFieldTwoPhaseSurfaceTension::precomputeQpResidual()
{
  return _coeff*_psi[_qp]*_grad_phi[_qp];
}
