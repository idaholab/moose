//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DamageBase.h"

template <bool is_ad>
InputParameters
DamageBaseTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Base class for damage models for use in conjunction with "
      "ComputeMultipleInelasticStress. The damage model updates the "
      "stress and Jacobian multiplier at the end of the stress computation.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  // The damage materials are designed to be called by another model, and not
  // called directly by MOOSE, so set compute=false.
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");
  return params;
}

template <bool is_ad>
DamageBaseTempl<is_ad>::DamageBaseTempl(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "")
{
}

template <bool is_ad>
void
DamageBaseTempl<is_ad>::setQp(unsigned int qp)
{
  _qp = qp;
}

template <bool is_ad>
void
DamageBaseTempl<is_ad>::updateDamage()
{
}

template <bool is_ad>
Real
DamageBaseTempl<is_ad>::computeTimeStepLimit()
{
  return std::numeric_limits<Real>::max();
}

template <bool is_ad>
void
DamageBaseTempl<is_ad>::finiteStrainRotation(
    const GenericRankTwoTensor<is_ad> & /*rotation_increment*/)
{
}

template class DamageBaseTempl<false>;
template class DamageBaseTempl<true>;
