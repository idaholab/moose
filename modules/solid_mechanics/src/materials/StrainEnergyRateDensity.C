//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StrainEnergyRateDensity.h"
#include "RankTwoTensor.h"
#include "MooseMesh.h"

registerMooseObject("SolidMechanicsApp", StrainEnergyRateDensity);
registerMooseObject("SolidMechanicsApp", ADStrainEnergyRateDensity);

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
  params.addParam<bool>("use_incremental_serd",
                        false,
                        "Use a trapezoidal incremental calculation of strain energy rate density.");
  params.addParam<std::vector<MaterialName>>(
      "inelastic_models",
      "The material objects to use to calculate the strain energy rate density.");
  return params;
}

template <bool is_ad>
StrainEnergyRateDensityTempl<is_ad>::StrainEnergyRateDensityTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _strain_energy_rate_density(declareProperty<Real>(_base_name + "strain_energy_rate_density")),
    _strain_energy_rate_density_old(
        getMaterialPropertyOld<Real>(_base_name + "strain_energy_rate_density")),
    _stress(getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "stress")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _strain_rate(getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "strain_rate")),
    _strain_rate_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "strain_rate")),
    _num_models(isParamValid("inelastic_models")
                    ? getParam<std::vector<MaterialName>>("inelastic_models").size()
                    : 0),
    _use_incremental_serd(getParam<bool>("use_incremental_serd"))
{
}

template <bool is_ad>
void
StrainEnergyRateDensityTempl<is_ad>::initialSetup()
{
  if (_use_incremental_serd)
  {
    if (_num_models)
      paramError("inelastic_models",
                 "inelastic_models cannot be used with use_incremental_serd=true.");
    return;
  }

  if (_num_models != 1)
    paramError("inelastic_models",
               "Specify exactly one inelastic model when use_incremental_serd=false.");

  const auto & models = getParam<std::vector<MaterialName>>("inelastic_models");

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
  if (_use_incremental_serd)
  {
    const Real effective_stress =
        std::sqrt(3.0 * MetaPhysicL::raw_value(_stress[_qp].secondInvariant()));
    const Real effective_stress_old = std::sqrt(3.0 * _stress_old[_qp].secondInvariant());
    const Real effective_strain_rate = std::sqrt(
        2.0 / 3.0 * MetaPhysicL::raw_value(_strain_rate[_qp].doubleContraction(_strain_rate[_qp])));
    const Real effective_strain_rate_old =
        std::sqrt(2.0 / 3.0 * _strain_rate_old[_qp].doubleContraction(_strain_rate_old[_qp]));

    _strain_energy_rate_density[_qp] = _strain_energy_rate_density_old[_qp] +
                                       0.5 * (effective_stress + effective_stress_old) *
                                           (effective_strain_rate - effective_strain_rate_old);
    return;
  }

  for (unsigned int i = 0; i < _inelastic_models.size(); ++i)
  {
    _inelastic_models[i]->setQp(_qp);
    _strain_energy_rate_density[_qp] = MetaPhysicL::raw_value(
        _inelastic_models[i]->computeStrainEnergyRateDensity(_stress, _strain_rate));
  }
}
