//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADViscoplasticityStressUpdateBase.h"

InputParameters
ADViscoplasticityStressUpdateBase::validParams()
{
  InputParameters params = ADStressUpdateBase::validParams();
  params.addClassDescription("Base class used to calculate viscoplastic responses to be used in "
                             "ADComputeMultiplePorousInelasticStress");
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

ADViscoplasticityStressUpdateBase::ADViscoplasticityStressUpdateBase(
    const InputParameters & parameters)
  : ADStressUpdateBase(parameters),
    _total_strain_base_name(isParamValid("total_strain_base_name")
                                ? getParam<std::string>("total_strain_base_name") + "_"
                                : ""),
    _strain_increment(
        getADMaterialProperty<RankTwoTensor>(_total_strain_base_name + "strain_increment")),
    _effective_inelastic_strain(declareADProperty<Real>(
        _base_name + "effective_" + getParam<std::string>("inelastic_strain_name"))),
    _effective_inelastic_strain_old(getMaterialPropertyOld<Real>(
        _base_name + "effective_" + getParam<std::string>("inelastic_strain_name"))),
    _inelastic_strain(declareADProperty<RankTwoTensor>(
        _base_name + getParam<std::string>("inelastic_strain_name"))),
    _inelastic_strain_old(getMaterialPropertyOld<RankTwoTensor>(
        _base_name + getParam<std::string>("inelastic_strain_name"))),
    _max_inelastic_increment(getParam<Real>("max_inelastic_increment")),
    _intermediate_porosity(0.0),
    _porosity_old(getMaterialPropertyOld<Real>(getParam<MaterialPropertyName>("porosity_name"))),
    _verbose(getParam<bool>("verbose")),
    _initial_porosity(getParam<Real>("initial_porosity")),
    _negative_behavior(this->template getParam<MooseEnum>("negative_behavior")
                           .template getEnum<NegativeBehavior>())
{
}

void
ADViscoplasticityStressUpdateBase::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
  _inelastic_strain[_qp].zero();
}

void
ADViscoplasticityStressUpdateBase::propagateQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp];
}

Real
ADViscoplasticityStressUpdateBase::computeTimeStepLimit()
{
  const Real scalar_inelastic_strain_incr =
      std::abs(MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
               _effective_inelastic_strain_old[_qp]);

  if (!scalar_inelastic_strain_incr)
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}

void
ADViscoplasticityStressUpdateBase::updateIntermediatePorosity(
    const ADRankTwoTensor & elastic_strain_increment)
{
  // Subtract elastic strain from strain increment to find all inelastic strain increments
  // calculated so far except the one that we're about to calculate. Then calculate intermdiate
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
