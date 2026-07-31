//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "PowerLawNumericalStrainEnergyRateDensity.h"

#include "libmesh/quadrature_gauss.h"

registerMooseObject("SolidMechanicsTestApp", PowerLawNumericalStrainEnergyRateDensity);

InputParameters
PowerLawNumericalStrainEnergyRateDensity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Computes power-law strain energy rate density using fifth-order Gaussian quadrature.");
  params.addRequiredRangeCheckedParam<Real>(
      "n_exponent", "n_exponent>0", "Exponent of the effective stress in the power-law model.");
  return params;
}

PowerLawNumericalStrainEnergyRateDensity::PowerLawNumericalStrainEnergyRateDensity(
    const InputParameters & parameters)
  : Material(parameters),
    _stress(getMaterialProperty<RankTwoTensor>("stress")),
    _strain_rate(getMaterialProperty<RankTwoTensor>("strain_rate")),
    _strain_energy_rate_density(declareProperty<Real>("strain_energy_rate_density")),
    _n_exponent(getParam<Real>("n_exponent"))
{
}

void
PowerLawNumericalStrainEnergyRateDensity::computeQpProperties()
{
  const Real effective_stress = std::sqrt(3.0 * _stress[_qp].secondInvariant());
  const Real effective_strain_rate =
      std::sqrt(2.0 / 3.0 * _strain_rate[_qp].doubleContraction(_strain_rate[_qp]));

  if (effective_stress == 0.0)
  {
    _strain_energy_rate_density[_qp] = 0.0;
    return;
  }

  const QGauss qrule(1, FIFTH);
  const auto & weights = qrule.get_weights();
  const auto & points = qrule.get_points();

  Real strain_rate_integral = 0.0;
  for (const auto i : index_range(points))
  {
    const Real normalized_stress = 0.5 * (points[i](0) + 1.0);
    strain_rate_integral += 0.5 * effective_stress * weights[i] * effective_strain_rate *
                            std::pow(normalized_stress, _n_exponent);
  }

  _strain_energy_rate_density[_qp] =
      effective_stress * effective_strain_rate - strain_rate_integral;
}
