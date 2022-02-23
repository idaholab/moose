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
  params.addRequiredRangeCheckedParam<std::vector<MaterialName>>(
      "inelastic_models",
      "inelastic_models_size=1",
      "The material objects to use to calculate the strain energy rate density.");
  return params;
}

template <bool is_ad>
StrainEnergyRateDensityTempl<is_ad>::StrainEnergyRateDensityTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _strain_energy_rate_density(declareProperty<Real>(_base_name + "strain_energy_rate_density")),
    _stress(getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "stress")),
    _strain_rate(getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "strain_rate")),
    _num_models(getParam<std::vector<MaterialName>>("inelastic_models").size())
{
}

template <bool is_ad>
void
StrainEnergyRateDensityTempl<is_ad>::initialSetup()
{
  std::vector<MaterialName> models = getParam<std::vector<MaterialName>>("inelastic_models");

  // Store inelastic models as generic StressUpdateBase.
  for (unsigned int i = 0; i < _num_models; ++i)
  {
    GenericStressUpdateBase<is_ad> * inelastic_model_stress_update =
        dynamic_cast<GenericStressUpdateBase<is_ad> *>(&getMaterialByName(models[i]));

    if (inelastic_model_stress_update)
      _inelastic_models.push_back(inelastic_model_stress_update);
  }
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
    _inelastic_models[i]->setQp(_qp);
    _strain_energy_rate_density[_qp] = MetaPhysicL::raw_value(
        _inelastic_models[i]->computeStrainEnergyRateDensity(_stress, _strain_rate));
  }
}
