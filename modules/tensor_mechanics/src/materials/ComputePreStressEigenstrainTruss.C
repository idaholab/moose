//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePreStressEigenstrainTruss.h"

registerMooseObject("TensorMechanicsApp", ComputePreStressEigenstrainTruss);

InputParameters
ComputePreStressEigenstrainTruss::validParams()
{
  InputParameters params = ComputeEigenstrainTrussBase::validParams();
  params.addRequiredParam<Real>("pre_stressing_strain", "pre stressing strain");
  return params;
}

ComputePreStressEigenstrainTruss::ComputePreStressEigenstrainTruss(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainTrussBase>(parameters),
    _pre_stressing_strain(getParam<Real>("pre_stressing_strain"))
{
}

void
ComputePreStressEigenstrainTruss::computeQpEigenstrain()
{
  _disp_eigenstrain[_qp].zero();
  _disp_eigenstrain[_qp](0) = _pre_stressing_strain;
}
