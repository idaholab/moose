//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThermalExpansionEigenstrainTrussBase.h"

InputParameters
ComputeThermalExpansionEigenstrainTrussBase::validParams()
{
  InputParameters params = ComputeEigenstrainTrussBase::validParams();
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addRequiredCoupledVar("stress_free_temperature",
                               "Reference temperature at which there is no "
                               "thermal expansion for thermal eigenstrain "
                               "calculation");
  return params;
}

ComputeThermalExpansionEigenstrainTrussBase::ComputeThermalExpansionEigenstrainTrussBase(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainTrussBase>(parameters),
    _temperature(coupledValue("temperature")),
    _stress_free_temperature(coupledValue("stress_free_temperature"))
{
}

void
ComputeThermalExpansionEigenstrainTrussBase::computeQpEigenstrain()
{
  Real thermal_strain = 0.0;
  computeThermalStrain(thermal_strain);
  _disp_eigenstrain[_qp].zero();
  _disp_eigenstrain[_qp](0) = thermal_strain;
}
