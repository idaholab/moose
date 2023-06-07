//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LarrysFit.h"

registerMooseObject("PhaseFieldTestApp", LarrysFit);

InputParameters
LarrysFit::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material that implements the a function of one variable and its first derivative.");
  params.addRequiredCoupledVar("porosity", "Porosity");
  params.addRequiredCoupledVar("Na_filled_fraction", "Na filled fraction");
  return params;
}

LarrysFit::LarrysFit(const InputParameters & parameters)
  : Material(parameters),
    // clang-format off
    _covariance(0.000033, -0.000347, -0.000128, 0.001111, 0.000597, 0.000164, -0.001128, -0.000725, -0.000285, -0.000069,
                -0.000347, 0.007733, 0.000597, -0.036611, -0.006508, -0.000285, 0.048592, 0.012426, 0.001424, -0.000000,
                -0.000128, 0.000597, 0.001597, -0.000725, -0.003202, -0.003783, -0.000000, 0.001812, 0.002848, 0.002480,
                0.001111, -0.036611, -0.000725, 0.211933, 0.012426, -0.000000, -0.322157, -0.031065, 0.000000, 0.000000,
                0.000597, -0.006508, -0.003202, 0.012426, 0.024813, 0.002848, -0.000000, -0.031065, -0.014238, 0.000000,
                0.000164, -0.000285, -0.003783, -0.000000, 0.002848, 0.011391, 0.000000, 0.000000, -0.003560, -0.008652,
                -0.001128, 0.048592, -0.000000, -0.322157, -0.000000, 0.000000, 0.536928, 0.000000, -0.000000, -0.000000,
                -0.000725, 0.012426, 0.001812, -0.031065, -0.031065, 0.000000, 0.000000, 0.077663, -0.000000, -0.000000,
                -0.000285, 0.001424, 0.002848, 0.000000, -0.014238, -0.003560, -0.000000, -0.000000, 0.017798, 0.000000,
                -0.000069, -0.000000, 0.002480, 0.000000, 0.000000, -0.008652, -0.000000, -0.000000, 0.000000, 0.007210),
    // clang-format on
    _porosity(makeRef(adCoupledValue("porosity"), _qp)),
    _na_filled_fraction(makeRef(adCoupledValue("Na_filled_fraction"), _qp)),
    _diff(declareADPropertyByName<Real>("Na_diff")),
    _stddev_diff(declareADPropertyByName<Real>("Na_diff_stddev"))
{
}

void
LarrysFit::computeQpProperties()
{
  const int parameter_tag = 0;
  const auto [p00, p10, p01, p20, p11, p02, p30, p21, p12, p03] =
      CompileTimeDerivatives::makeValues<parameter_tag>(0.002081,
                                                        0.537067,
                                                        0.043118,
                                                        -3.129198,
                                                        -1.216443,
                                                        0.014718,
                                                        6.026397,
                                                        4.144416,
                                                        0.523929,
                                                        -0.052894);

  const auto & x = _porosity;
  const auto & y = _na_filled_fraction;

  // compute diffusivity
  const auto diff = p00 + p10 * x + p01 * y + p20 * x * x + p11 * x * y + p02 * y * y +
                    p30 * x * x * x + p21 * x * x * y + p12 * x * y * y + p03 * y * y * y;
  _diff[_qp] = diff();

  // compute standard deviation
  const auto stddev =
      CompileTimeDerivatives::makeStandardDeviation<parameter_tag>(diff, _covariance);
  _stddev_diff[_qp] = stddev();
}
