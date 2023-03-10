//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIdealGasPressure.h"

registerMooseObject("TensorMechanicsApp", ComputeIdealGasPressure);

InputParameters
ComputeIdealGasPressure::validParams()
{
  auto params = ComputeGasPressureBase::validParams();
  params.addClassDescription(
      "Computes a isotropic extra Stress with a trace that equals the pressure of an ideal gas");
  return params;
}

ComputeIdealGasPressure::ComputeIdealGasPressure(const InputParameters & parameters)
  : ComputeGasPressureBase(parameters)
{
}

void
ComputeIdealGasPressure::computeQpExtraStress()
{
  /**
   * number density:
   *   n = N/V = c/omega
   *
   * Ideal gas pressure:
   *   PV = N*k_B*T
   *   P = N/V*k_B*T
   *   P = n*k_B*T
   */
  const Real n = _c[_qp] / _omega;
  const Real P = n * _kB * _T[_qp] * _pressure_unit_conversion;

  _extra_stress[_qp].zero();
  _extra_stress[_qp](0, 0) = -P;
  _extra_stress[_qp](1, 1) = -P;
  _extra_stress[_qp](2, 2) = -P;
}
