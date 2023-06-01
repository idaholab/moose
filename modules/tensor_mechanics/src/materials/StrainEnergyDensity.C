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
registerMooseObject("TensorMechanicsApp", ADStrainEnergyDensity);

template <bool is_ad>
InputParameters
StrainEnergyDensityTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the strain energy density using a combination of the "
                             "elastic and inelastic components of the strain increment, which is a "
                             "valid assumption for monotonic behavior.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<bool>(
      "incremental",
      "Optional flag for error checking if an incremental or total model should be used.");
  return params;
}

template <bool is_ad>
StrainEnergyDensityTempl<is_ad>::StrainEnergyDensityTempl(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _strain_energy_density(declareProperty<Real>(_base_name + "strain_energy_density")),
    _strain_energy_density_old(getMaterialPropertyOld<Real>(_base_name + "strain_energy_density")),
    _stress(getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "stress")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _mechanical_strain(
        getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "mechanical_strain")),
    _strain_increment(
        getGenericOptionalMaterialProperty<RankTwoTensor, is_ad>(_base_name + "strain_increment"))
{
}

template <bool is_ad>
void
StrainEnergyDensityTempl<is_ad>::initialSetup()
{
  // optional error checking
  if (isParamValid("incremental"))
  {
    auto incremental = getParam<bool>("incremental");
    if (incremental && !_strain_increment)
      mooseError("StrainEnergyDensity: Specified incremental = true, but material model is "
                 "not incremental.");
    if (!incremental && _strain_increment)
      mooseError("StrainEnergyDensity: Specified incremental = false, but material model is "
                 "incremental.");
  }
}
template <bool is_ad>
void
StrainEnergyDensityTempl<is_ad>::initQpStatefulProperties()
{
  _strain_energy_density[_qp] = 0.0;
}

template <bool is_ad>
void
StrainEnergyDensityTempl<is_ad>::computeQpProperties()
{

  if (_strain_increment)
    _strain_energy_density[_qp] =
        _strain_energy_density_old[_qp] +
        MetaPhysicL::raw_value(_stress[_qp])
                .doubleContraction(MetaPhysicL::raw_value(_strain_increment[_qp])) /
            2.0 +
        _stress_old[_qp].doubleContraction(MetaPhysicL::raw_value(_strain_increment[_qp])) / 2.0;
  else
    _strain_energy_density[_qp] =
        MetaPhysicL::raw_value(_stress[_qp])
            .doubleContraction((MetaPhysicL::raw_value(_mechanical_strain[_qp]))) /
        2.0;
}

template class StrainEnergyDensityTempl<false>;
template class StrainEnergyDensityTempl<true>;
