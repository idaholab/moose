//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StrainEnergyDensity.h"
#include "RankTwoTensor.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", StrainEnergyDensity);

template <>
InputParameters
validParams<StrainEnergyDensity>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Computes the strain energy density using a combination of the "
                             "elastic and inelastic components of the strain increment, which is a "
                             "valid assumption for monotonic behavior.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addRequiredParam<bool>(
      "incremental", "Flag to indicate whether an incremental or total model is being used.");
  return params;
}

StrainEnergyDensity::StrainEnergyDensity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _incremental(getParam<bool>("incremental")),
    _strain_energy_density(declareProperty<Real>(_base_name + "strain_energy_density")),
    _strain_energy_density_old(getMaterialPropertyOld<Real>(_base_name + "strain_energy_density")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _mechanical_strain(getMaterialProperty<RankTwoTensor>(_base_name + "mechanical_strain")),
    _strain_increment(
        _incremental ? &getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment") : NULL)
{
}

void
StrainEnergyDensity::initialSetup()
{
  if (!_incremental && hasMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
    mooseError("StrainEnergyDensity: Specified incremental = false, but material model is "
               "incremental.");
}

void
StrainEnergyDensity::initQpStatefulProperties()
{
  _strain_energy_density[_qp] = 0.0;
}

void
StrainEnergyDensity::computeQpProperties()
{

  if (_strain_increment != nullptr)
    _strain_energy_density[_qp] =
        _strain_energy_density_old[_qp] +
        _stress[_qp].doubleContraction((*_strain_increment)[_qp]) / 2.0 +
        _stress_old[_qp].doubleContraction((*_strain_increment)[_qp]) / 2.0;
  else
    _strain_energy_density[_qp] = _stress[_qp].doubleContraction((_mechanical_strain)[_qp]) / 2.0;
}
