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
registerMooseObject("TensorMechanicsApp", ADStrainEnergyRateDensity);

template <bool is_ad>
InputParameters
StrainEnergyRateDensityTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the strain energy density rate using a combination of the "
                             "elastic and inelastic components of the strain increment, which is a "
                             "valid assumption for monotonic behavior.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addRequiredParam<std::vector<MaterialName>>(
      "inelastic_models",
      "The material objects to use to calculate the strain energy rate density.");
  return params;
}

template <bool is_ad>
StrainEnergyRateDensityTempl<is_ad>::StrainEnergyRateDensityTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _strain_energy_rate_density(
        declareGenericProperty<Real, is_ad>(_base_name + "strain_energy_rate_density")),
    _stress(getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "stress")),
    _strain_rate(getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "strain_rate")),
    _num_models(getParam<std::vector<MaterialName>>("inelastic_models").size())
{

  std::vector<MaterialName> models = getParam<std::vector<MaterialName>>("inelastic_models");

  if (_num_models > 1)
    paramError("inelastic_models",
               "StrainEnergyRateDensity can only compute the strain energy release rate density"
               " when one inelastic model is supplied. ");

  if (_num_models < 1)
    paramError("inelastic_models",
               "StrainEnergyRateDensity requires one inelastic model to be supplied. ");

  // Store AD and non-AD inelastic models separately
  for (unsigned int i = 0; i < _num_models; ++i)
  {
    RadialReturnCreepStressUpdateBase * inelastic_model_stress_update =
        dynamic_cast<RadialReturnCreepStressUpdateBase *>(&getMaterialByName(models[i]));

    if (inelastic_model_stress_update)
    {
      _inelastic_models.push_back(inelastic_model_stress_update);
      continue;
    }

    ADRadialReturnCreepStressUpdateBase * ad_inelastic_model_stress_update =
        dynamic_cast<ADRadialReturnCreepStressUpdateBase *>(&getMaterialByName(models[i]));

    if (ad_inelastic_model_stress_update)
      _ad_inelastic_models.push_back(ad_inelastic_model_stress_update);
  }
}

template <bool is_ad>
void
StrainEnergyRateDensityTempl<is_ad>::initialSetup()
{
}

template <bool is_ad>
void
StrainEnergyRateDensityTempl<is_ad>::initQpStatefulProperties()
{
  _strain_energy_rate_density[_qp] = 0.0;
}

template <bool is_ad>
void
StrainEnergyRateDensityTempl<is_ad>::computeQpProperties()
{

  for (unsigned int i = 0; i < _inelastic_models.size(); ++i)
  {
    MaterialProperty<Real> * strain_energy_rate_density =
        dynamic_cast<MaterialProperty<Real> *>(&_strain_energy_rate_density);

    const MaterialProperty<RankTwoTensor> * stress =
        dynamic_cast<const MaterialProperty<RankTwoTensor> *>(&_stress);

    const MaterialProperty<RankTwoTensor> * strain_rate =
        dynamic_cast<const MaterialProperty<RankTwoTensor> *>(&_strain_rate);

    if (strain_energy_rate_density && stress && strain_rate)
      _inelastic_models[i]->computeStrainEnergyRateDensity(
          *strain_energy_rate_density, *stress, *strain_rate);
  }

  for (unsigned int i = 0; i < _ad_inelastic_models.size(); ++i)
  {
    ADMaterialProperty<Real> * ad_strain_energy_rate_density =
        dynamic_cast<ADMaterialProperty<Real> *>(&_strain_energy_rate_density);

    const ADMaterialProperty<RankTwoTensor> * ad_stress =
        dynamic_cast<const ADMaterialProperty<RankTwoTensor> *>(&_stress);

    const ADMaterialProperty<RankTwoTensor> * ad_strain_rate =
        dynamic_cast<const ADMaterialProperty<RankTwoTensor> *>(&_strain_rate);

    if (ad_strain_energy_rate_density && ad_stress && ad_strain_rate)
      _ad_inelastic_models[i]->computeStrainEnergyRateDensity(
          *ad_strain_energy_rate_density, *ad_stress, *ad_strain_rate);
  }
}
