//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StrainEnergyRateDensity.h"
#include "RankTwoTensor.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", StrainEnergyRateDensity);

InputParameters
StrainEnergyRateDensity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the strain energy density rate using a combination of the "
                             "elastic and inelastic components of the strain increment, which is a "
                             "valid assumption for monotonic behavior.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");

  return params;
}

StrainEnergyRateDensity::StrainEnergyRateDensity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _strain_energy_rate_density(declareProperty<Real>(_base_name + "strain_energy_rate_density")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _strain_rate(getMaterialProperty<RankTwoTensor>(_base_name + "strain_rate")),
    _n_exponent(isParamValid("n_exponent") ? getParam<Real>("n_exponent") : -1)
{
}

void
StrainEnergyRateDensity::initialSetup()
{
}

void
StrainEnergyRateDensity::initQpStatefulProperties()
{
  _strain_energy_rate_density[_qp] = 0.0;
}

void
StrainEnergyRateDensity::computeQpProperties()
{
  Real creep_factor;

  if (_n_exponent > 0)
    creep_factor = _n_exponent / (_n_exponent + 1);
  else
    creep_factor = 1.0;

  _strain_energy_rate_density[_qp] =
      creep_factor * _stress[_qp].doubleContraction((_strain_rate)[_qp]);
}
