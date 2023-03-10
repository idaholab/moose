//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeVanDerWaalsGasPressure.h"

registerMooseObject("TensorMechanicsApp", ComputeVanDerWaalsGasPressure);

InputParameters
ComputeVanDerWaalsGasPressure::validParams()
{
  auto params = ComputeGasPressureBase::validParams();
  params.addClassDescription(
      "Computes a isotropic extra Stress with a trace that equals the pressure of an ideal gas");
  params.addRequiredParam<Real>("a",
                                "Van der Waals coefficient a (default pressure_unit_conversion "
                                "requires this to be in [eV*Ang^3])");
  params.addRequiredParam<Real>("b",
                                "Van der Waals molecular exclusion volume b (default "
                                "pressure_unit_conversion requires this to be in [Ang^3])");
  return params;
}

ComputeVanDerWaalsGasPressure::ComputeVanDerWaalsGasPressure(const InputParameters & parameters)
  : ComputeGasPressureBase(parameters), _a(getParam<Real>("a")), _b(getParam<Real>("b"))
{
}

void
ComputeVanDerWaalsGasPressure::computeQpExtraStress()
{
  /**
   * number density:
   *   n = N/V = c/omega
   *
   * Van der Waals pressure:
   *   (P + aN^2/V^2)(V - Nb) = N*k_B*T
   *   (P + aN^2/V^2)(1 - N/V*b) = N/V*k_b*T
   *   (P + aN^2/V^2) = N/V*k_b*T / (1 - N/V*b)
   *   (P + a n^2) = n*k_b*T / (1 - n*b)
   *   P = n*k_b*T / (1 - n*b) - a n^2
   */

  const Real n = _c[_qp] / _omega;
  const Real P = (n * _kB * _T[_qp] / (1.0 - n * _b) - _a * n * n) * _pressure_unit_conversion;

  _extra_stress[_qp].zero();
  _extra_stress[_qp](0, 0) = -P;
  _extra_stress[_qp](1, 1) = -P;
  _extra_stress[_qp](2, 2) = -P;
}
