//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThermalExpansionEigenstrainBeamBase.h"

template <>
InputParameters
validParams<ComputeThermalExpansionEigenstrainBeamBase>()
{
  InputParameters params = validParams<ComputeEigenstrainBeamBase>();
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addRequiredCoupledVar("stress_free_temperature",
                               "Reference temperature at which there is no "
                               "thermal expansion for thermal eigenstrain "
                               "calculation");
  return params;
}

ComputeThermalExpansionEigenstrainBeamBase::ComputeThermalExpansionEigenstrainBeamBase(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainBeamBase>(parameters),
    _temperature(coupledValue("temperature")),
    _stress_free_temperature(coupledValue("stress_free_temperature")),
    _initial_axis(declareProperty<RealGradient>("initial_axis"))
{
}

void
ComputeThermalExpansionEigenstrainBeamBase::initQpStatefulProperties()
{
  // compute initial orientation of the beam
  const std::vector<RealGradient> * orientation =
      &_subproblem.assembly(_tid).getFE(FEType(), 1)->get_dxyzdxi();

  RealGradient x_orientation = (*orientation)[0];
  x_orientation /= x_orientation.norm();

  _initial_axis[_qp] = x_orientation;
}
void
ComputeThermalExpansionEigenstrainBeamBase::computeQpEigenstrain()
{
  Real thermal_strain = 0.0;
  Real instantaneous_cte = 0.0;

  computeThermalStrain(thermal_strain, instantaneous_cte);

  _disp_eigenstrain[_qp].zero();
  _rot_eigenstrain[_qp].zero();
  _disp_eigenstrain[_qp] = _initial_axis[_qp] * thermal_strain;
}
