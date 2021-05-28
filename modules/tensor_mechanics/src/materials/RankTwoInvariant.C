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
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Name of the material property computed by this model");
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
    _invariant(
        getParam<MooseEnum>("invariant").template getEnum<RankTwoScalarTools::InvariantType>()),
    _stateful(_invariant == RankTwoScalarTools::InvariantType::EffectiveStrain),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _tensor_old(_stateful ? &getMaterialPropertyOld<RankTwoTensor>("rank_two_tensor") : nullptr),
    _property(declareGenericProperty<Real, is_ad>("property_name")),
    _property_old(_stateful ? &getMaterialPropertyOld<Real>("property_name") : nullptr)
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
    case RankTwoScalarTools::InvariantType::MaxPrincipal:
    case RankTwoScalarTools::InvariantType::MidPrincipal:
    case RankTwoScalarTools::InvariantType::MinPrincipal:
    {
      Point dummy_direction;
      _property[_qp] = RankTwoScalarTools::getPrincipalComponent(
          MetaPhysicL::raw_value(_tensor[_qp]), _invariant, dummy_direction);
      break;
    }

    case RankTwoScalarTools::InvariantType::VonMisesStress:
    case RankTwoScalarTools::InvariantType::Hydrostatic:
    case RankTwoScalarTools::InvariantType::L2norm:
    case RankTwoScalarTools::InvariantType::VolumetricStrain:
    case RankTwoScalarTools::InvariantType::FirstInvariant:
    case RankTwoScalarTools::InvariantType::SecondInvariant:
    case RankTwoScalarTools::InvariantType::ThirdInvariant:
    case RankTwoScalarTools::InvariantType::TriaxialityStress:
    case RankTwoScalarTools::InvariantType::MaxShear:
    case RankTwoScalarTools::InvariantType::StressIntensity:
    {
      _property[_qp] =
          RankTwoScalarTools::getInvariant(MetaPhysicL::raw_value(_tensor[_qp]), _invariant);
      break;
    }

    case RankTwoScalarTools::InvariantType::EffectiveStrain:
    {
      mooseAssert(_tensor_old, "The selected invariant requires the input material to be stateful");
      mooseAssert(_property_old,
                  "The selected invariant requires the output material to be stateful");
      _property[_qp] = (*_property_old)[_qp] + RankTwoScalarTools::effectiveStrain(
                                                   MetaPhysicL::raw_value(_tensor[_qp]) -
                                                   MetaPhysicL::raw_value((*_tensor_old)[_qp]));
      break;
    }

    default:
      mooseError("Not a recognized invariant for RankTwoInvariant");
  }
}

template class RankTwoInvariantTempl<false>;
template class RankTwoInvariantTempl<true>;
