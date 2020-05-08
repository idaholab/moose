//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoInvariant.h"
#include "RankTwoScalarTools.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", RankTwoInvariant);
registerMooseObject("TensorMechanicsApp", ADRankTwoInvariant);

template <bool is_ad>
InputParameters
RankTwoInvariantTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a invariant property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addRequiredParam<std::string>("property_name",
                                       "Name of the material property computed by this model");
  MooseEnum mixedInvariants(
      "VonMisesStress EffectiveStrain Hydrostatic L2norm VolumetricStrain FirstInvariant "
      "SecondInvariant ThirdInvariant TriaxialityStress MaxShear StressIntensity MaxPrincipal "
      "MidPrincipal MinPrincipal");

  params.addParam<MooseEnum>("invariant", mixedInvariants, "Type of invariant output");

  return params;
}

template <bool is_ad>
RankTwoInvariantTempl<is_ad>::RankTwoInvariantTempl(const InputParameters & parameters)
  : Material(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _property_name(
        isParamValid("property_name") ? this->template getParam<std::string>("property_name") : ""),
    _property(declareGenericProperty<Real, is_ad>(_property_name)),
    _invariant(
        getParam<MooseEnum>("invariant").template getEnum<RankTwoScalarTools::INVARIANT_TYPE>())
{
}

template <bool is_ad>
void
RankTwoInvariantTempl<is_ad>::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

template <bool is_ad>
void
RankTwoInvariantTempl<is_ad>::computeQpProperties()
{
  switch (_invariant)
  {
    case RankTwoScalarTools::INVARIANT_TYPE::MaxPrincipal:
    case RankTwoScalarTools::INVARIANT_TYPE::MidPrincipal:
    case RankTwoScalarTools::INVARIANT_TYPE::MinPrincipal:
    {
      Point dummy_direction;
      _property[_qp] = RankTwoScalarTools::getPrincipalComponent(
          MetaPhysicL::raw_value(_tensor[_qp]), _invariant, dummy_direction);
      break;
    }
    case RankTwoScalarTools::INVARIANT_TYPE::VonMisesStress:
    case RankTwoScalarTools::INVARIANT_TYPE::EffectiveStrain:
    case RankTwoScalarTools::INVARIANT_TYPE::Hydrostatic:
    case RankTwoScalarTools::INVARIANT_TYPE::L2norm:
    case RankTwoScalarTools::INVARIANT_TYPE::VolumetricStrain:
    case RankTwoScalarTools::INVARIANT_TYPE::FirstInvariant:
    case RankTwoScalarTools::INVARIANT_TYPE::SecondInvariant:
    case RankTwoScalarTools::INVARIANT_TYPE::ThirdInvariant:
    case RankTwoScalarTools::INVARIANT_TYPE::TriaxialityStress:
    case RankTwoScalarTools::INVARIANT_TYPE::MaxShear:
    case RankTwoScalarTools::INVARIANT_TYPE::StressIntensity:
    {
      _property[_qp] =
          RankTwoScalarTools::getInvariant(MetaPhysicL::raw_value(_tensor[_qp]), _invariant);
      break;
    }
    default:
      mooseError("Not a recognized invariant for RankTwoInvariant");
  }
}

template class RankTwoInvariantTempl<false>;
template class RankTwoInvariantTempl<true>;
