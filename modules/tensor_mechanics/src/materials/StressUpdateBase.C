//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressUpdateBase.h"

#include "MooseMesh.h"

template <bool is_ad, typename R2, typename R4>
InputParameters
StressUpdateBaseTempl<is_ad, R2, R4>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculates an admissible state (stress that lies on or within the "
                             "yield surface, plastic strains, internal parameters, etc).  This "
                             "class is intended to be a parent class for classes with specific "
                             "constitutive models.");
  params.addParam<std::string>(
      "base_name",
      "Optional parameter that defines a prefix for all material "
      "properties related to this stress update model. This allows for "
      "multiple models of the same type to be used without naming conflicts.");
  // The return stress increment classes are intended to be iterative materials, so must set compute
  // = false for all inheriting classes
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");
  return params;
}

template <bool is_ad, typename R2, typename R4>
StressUpdateBaseTempl<is_ad, R2, R4>::StressUpdateBaseTempl(const InputParameters & parameters)
  : Material(parameters),
    _base_name(this->isParamValid("base_name")
                   ? this->template getParam<std::string>("base_name") + "_"
                   : "")
{
}

template <bool is_ad, typename R2, typename R4>
void
StressUpdateBaseTempl<is_ad, R2, R4>::setQp(unsigned int qp)
{
  _qp = qp;
}

template <bool is_ad, typename R2, typename R4>
void
StressUpdateBaseTempl<is_ad, R2, R4>::propagateQpStatefulProperties()
{
  mooseError(
      "propagateQpStatefulProperties called: it needs to be implemented by your inelastic model");
}

template <bool is_ad, typename R2, typename R4>
Real
StressUpdateBaseTempl<is_ad, R2, R4>::computeTimeStepLimit()
{
  return std::numeric_limits<Real>::max();
}

template <bool is_ad, typename R2, typename R4>
void
StressUpdateBaseTempl<is_ad, R2, R4>::updateState(
    GR2 & /*strain_increment*/,
    GR2 & /*inelastic_strain_increment*/,
    const GR2 & /*rotation_increment*/,
    GR2 & /*stress_new*/,
    const RankTwoTensor & /*stress_old*/,
    const GR4 & /*elasticity_tensor*/,
    const RankTwoTensor & /*elastic_strain_old*/,
    bool /*compute_full_tangent_operator = false*/,
    RankFourTensor & /*tangent_operator = _identityTensor*/)
{
  mooseError("updateState called: it needs to be implemented by your inelastic model");
}

template <bool is_ad, typename R2, typename R4>
void
StressUpdateBaseTempl<is_ad, R2, R4>::updateStateSubstep(
    GR2 & /*strain_increment*/,
    GR2 & /*inelastic_strain_increment*/,
    const GR2 & /*rotation_increment*/,
    GR2 & /*stress_new*/,
    const RankTwoTensor & /*stress_old*/,
    const GR4 & /*elasticity_tensor*/,
    const RankTwoTensor & /*elastic_strain_old*/,
    bool /*compute_full_tangent_operator*/,
    RankFourTensor & /*tangent_operator*/)
{
  this->template paramError(
      "use_substep",
      "updateStateSubstep called: it needs to be implemented by your inelastic model");
}

template <bool is_ad, typename R2, typename R4>
TangentCalculationMethod
StressUpdateBaseTempl<is_ad, R2, R4>::getTangentCalculationMethod()
{
  return TangentCalculationMethod::ELASTIC;
}

template <>
TangentCalculationMethod
StressUpdateBaseTempl<true>::getTangentCalculationMethod()
{
  mooseError(
      "getTangentCalculationMethod called: no tangent moduli calculation is needed while using AD");
  return TangentCalculationMethod::ELASTIC;
}

template class StressUpdateBaseTempl<false>;
template class StressUpdateBaseTempl<true>;
