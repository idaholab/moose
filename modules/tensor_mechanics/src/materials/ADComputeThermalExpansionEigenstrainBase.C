//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeThermalExpansionEigenstrainBase.h"
#include "RankTwoTensor.h"

defineADValidParams(ADComputeThermalExpansionEigenstrainBase,
                    ADComputeEigenstrainBase,
                    params.addCoupledVar("temperature", "Coupled temperature");
                    params.addRequiredCoupledVar("stress_free_temperature",
                                                 "Reference temperature at which there is no "
                                                 "thermal expansion for thermal eigenstrain "
                                                 "calculation"););

template <ComputeStage compute_stage>
ADComputeThermalExpansionEigenstrainBase<compute_stage>::ADComputeThermalExpansionEigenstrainBase(
    const InputParameters & parameters)
  : ADComputeEigenstrainBase<compute_stage>(parameters),
    _temperature(adCoupledValue("temperature")),
    _deigenstrain_dT(declareADProperty<RankTwoTensor>(
        derivativePropertyNameFirst(_eigenstrain_name, this->getVar("temperature", 0)->name()))),
    _stress_free_temperature(adCoupledValue("stress_free_temperature"))
{
}

template <ComputeStage compute_stage>
void
ADComputeThermalExpansionEigenstrainBase<compute_stage>::computeQpEigenstrain()
{
  ADReal thermal_strain = 0.0;
  ADReal instantaneous_cte = 0.0;

  computeThermalStrain(thermal_strain, instantaneous_cte);

  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(thermal_strain);

  _deigenstrain_dT[_qp].zero();
  _deigenstrain_dT[_qp].addIa(instantaneous_cte);
}

adBaseClass(ADComputeThermalExpansionEigenstrainBase);
