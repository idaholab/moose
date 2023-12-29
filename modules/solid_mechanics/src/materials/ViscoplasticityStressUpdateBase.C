//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViscoplasticityStressUpdateBase.h"

template <bool is_ad>
InputParameters
ViscoplasticityStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = StressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription("Base class used to calculate viscoplastic responses to be used in "
                             "ComputeMultiplePorousInelasticStress");
  params.addParam<Real>("max_inelastic_increment",
                        1.0e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  params.addParam<std::string>(
      "inelastic_strain_name",
      "viscoplasticity",
      "Name of the material property that stores the effective inelastic strain");
  params.addParam<bool>("verbose", false, "Flag to output verbose information");
  params.addParam<MaterialPropertyName>(
      "porosity_name", "porosity", "Name of porosity material property");
  params.addParam<std::string>("total_strain_base_name", "Base name for the total strain");
  params.addRangeCheckedParam<Real>(
      "initial_porosity", 0.0, "initial_porosity>=0.0 & initial_porosity<1.0", "Initial porosity");
  MooseEnum negative_behavior("ZERO INITIAL_CONDITION EXCEPTION", "INITIAL_CONDITION");
  params.addParam<MooseEnum>(
      "negative_behavior", negative_behavior, "Enum how to handle negative porosities");

  params.addParamNamesToGroup("inelastic_strain_name", "Advanced");
  return params;
}

template <bool is_ad>
ViscoplasticityStressUpdateBaseTempl<is_ad>::ViscoplasticityStressUpdateBaseTempl(
    const InputParameters & parameters)
  : StressUpdateBaseTempl<is_ad>(parameters),
    _total_strain_base_name(this->isParamValid("total_strain_base_name")
                                ? this->template getParam<std::string>("total_strain_base_name") +
                                      "_"
                                : ""),
    _strain_increment(this->template getGenericMaterialProperty<RankTwoTensor, is_ad>(
        _total_strain_base_name + "strain_increment")),
    _effective_inelastic_strain(this->template declareGenericProperty<Real, is_ad>(
        _base_name + "effective_" + this->template getParam<std::string>("inelastic_strain_name"))),
    _effective_inelastic_strain_old(this->template getMaterialPropertyOld<Real>(
        _base_name + "effective_" + this->template getParam<std::string>("inelastic_strain_name"))),
    _inelastic_strain(this->template declareGenericProperty<RankTwoTensor, is_ad>(
        _base_name + this->template getParam<std::string>("inelastic_strain_name"))),
    _inelastic_strain_old(this->template getMaterialPropertyOld<RankTwoTensor>(
        _base_name + this->template getParam<std::string>("inelastic_strain_name"))),
    _max_inelastic_increment(this->template getParam<Real>("max_inelastic_increment")),
    _intermediate_porosity(0.0),
    _porosity_old(this->template getMaterialPropertyOld<Real>("porosity_name")),
    _verbose(this->template getParam<bool>("verbose")),
    _initial_porosity(this->template getParam<Real>("initial_porosity")),
    _negative_behavior(this->template getParam<MooseEnum>("negative_behavior")
                           .template getEnum<NegativeBehavior>())
{
}

template <bool is_ad>
void
ViscoplasticityStressUpdateBaseTempl<is_ad>::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
  _inelastic_strain[_qp].zero();
}

template <bool is_ad>
void
ViscoplasticityStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp];
}

template <bool is_ad>
Real
ViscoplasticityStressUpdateBaseTempl<is_ad>::computeTimeStepLimit()
{
  const Real scalar_inelastic_strain_incr =
      std::abs(MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
               _effective_inelastic_strain_old[_qp]);

  if (!scalar_inelastic_strain_incr)
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}

template <bool is_ad>
void
ViscoplasticityStressUpdateBaseTempl<is_ad>::updateIntermediatePorosity(
    const GenericRankTwoTensor<is_ad> & elastic_strain_increment)
{
  // Subtract elastic strain from strain increment to find all inelastic strain increments
  // calculated so far except the one that we're about to calculate. Then calculate intermediate
  // porosity from all inelastic strain increments calculated so far except the one that we're about
  // to calculate
  _intermediate_porosity =
      (1.0 - _porosity_old[_qp]) * (_strain_increment[_qp] - elastic_strain_increment).trace() +
      _porosity_old[_qp];

  if (_intermediate_porosity < 0.0)
  {
    if (_negative_behavior == NegativeBehavior::ZERO)
      _intermediate_porosity = 0.0;
    if (_negative_behavior == NegativeBehavior::INITIAL_CONDITION)
      _intermediate_porosity = _initial_porosity;
    if (_negative_behavior == NegativeBehavior::EXCEPTION)
      mooseException("In ", _name, ": porosity is negative.");
  }

  if (std::isnan(_intermediate_porosity))
    mooseException("In ", _name, ": porosity is nan. Cutting timestep.");
}

template class ViscoplasticityStressUpdateBaseTempl<false>;
template class ViscoplasticityStressUpdateBaseTempl<true>;
