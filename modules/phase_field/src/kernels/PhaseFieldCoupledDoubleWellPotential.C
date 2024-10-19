/****************************************************************************/
/*                        DO NOT MODIFY THIS HEADER                         */
/*                                                                          */
/* MALAMUTE: MOOSE Application Library for Advanced Manufacturing UTilitiEs */
/*                                                                          */
/*           Copyright 2021 - 2023, Battelle Energy Alliance, LLC           */
/*                           ALL RIGHTS RESERVED                            */
/****************************************************************************/

#include "PhaseFieldCoupledDoubleWellPotential.h"

registerMooseObject("PhaseFieldApp", PhaseFieldCoupledDoubleWellPotential);

InputParameters
PhaseFieldCoupledDoubleWellPotential::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addRequiredCoupledVar("c", "phase field variable");
  params.addRequiredParam<Real>("prefactor", "prefactor for double well potential");
  return params;
}

PhaseFieldCoupledDoubleWellPotential::PhaseFieldCoupledDoubleWellPotential(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _c(adCoupledValue("c")),
    _prefactor(getParam<Real>("prefactor"))


{
}

ADReal
PhaseFieldCoupledDoubleWellPotential::precomputeQpResidual()
{
  return _prefactor*_c[_qp]*(_c[_qp]*_c[_qp] - 1);
}
 