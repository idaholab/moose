//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADViscoplasticityStressUpdate.h"

#include "libmesh/utility.h"

registerADMooseObject("TensorMechanicsApp", ADViscoplasticityStressUpdate);

defineADValidParams(
    ADViscoplasticityStressUpdate,
    ADStressUpdateBase,
    params += validParams<ADSingleVariableReturnMappingSolution<RESIDUAL>>();
    params.addClassDescription(
        "This material computes the non-linear homogenized gauge stress in order to compute the "
        "viscoplastic responce due to creep in porous materials. This material must be used in "
        "conjunction with ComputeMultiplePorousInelasticStress");
    params.addParam<MooseEnum>("viscoplasticity_model",
                               ADViscoplasticityStressUpdate<RESIDUAL>::getModelEnum(),
                               "Which viscoplastic model to use");
    params.addParam<MooseEnum>("pore_shape_model",
                               ADViscoplasticityStressUpdate<RESIDUAL>::getPoreShapeEnum(),
                               "Which pore shape model to use");
    params.addParam<Real>("max_inelastic_increment",
                          1.0e-4,
                          "The maximum inelastic strain increment allowed in a time step");
    params.addParam<std::string>(
        "effective_inelastic_strain_name",
        "effective_viscoplasticity",
        "Name of the material property that stores the effective inelastic strain");
    params.addRequiredParam<MaterialPropertyName>(
        "coefficient", "Material property name for the leading coefficient for Norton power law");
    params.addRequiredRangeCheckedParam<Real>("power",
                                              "power>=1.0",
                                              "Stress exponent for Norton power law");
    params.addParam<bool>("verbose", false, "Flag to output verbose information");
    params.addParam<MaterialPropertyName>("porosity_name",
                                          "porosity",
                                          "Name of porosity material property");
    params.addParam<std::string>("total_strain_base_name", "Base name for the total strain");

    params.addParamNamesToGroup("effective_inelastic_strain_name", "Advanced"););

template <ComputeStage compute_stage>
ADViscoplasticityStressUpdate<compute_stage>::ADViscoplasticityStressUpdate(
    const InputParameters & parameters)
  : ADStressUpdateBase<compute_stage>(parameters),
    ADSingleVariableReturnMappingSolution<compute_stage>(parameters),
    _model(getParam<MooseEnum>("viscoplasticity_model")),
    _pore_shape(getParam<MooseEnum>("pore_shape_model")),
    _pore_shape_factor(_pore_shape == "spherical" ? 1.5 : std::sqrt(3.0)),
    _total_strain_base_name(isParamValid("total_strain_base_name")
                                ? getParam<std::string>("total_strain_base_name") + "_"
                                : ""),
    _strain_increment(
        getADMaterialProperty<RankTwoTensor>(_total_strain_base_name + "strain_increment")),
    _effective_inelastic_strain(declareADProperty<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _effective_inelastic_strain_old(getMaterialPropertyOld<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _creep_strain(declareADProperty<RankTwoTensor>(_base_name + "creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "creep_strain")),
    _max_inelastic_increment(getParam<Real>("max_inelastic_increment")),
    _power(getParam<Real>("power")),
    _power_factor(_model == "LPS" ? (_power - 1.0) / (_power + 1.0) : 1.0),
    _coefficient(getADMaterialProperty<Real>("coefficient")),
    _gauge_stress(declareADProperty<Real>(_base_name + "gauge_stress")),
    _intermediate_porosity(0.0),
    _porosity_old(getMaterialPropertyOld<Real>(getParam<MaterialPropertyName>("porosity_name"))),
    _verbose(getParam<bool>("verbose")),
    _hydro_stress(0.0),
    _identity_two(RankTwoTensor::initIdentity),
    _dhydro_stress_dsigma(_identity_two / 3.0),
    _derivative(0.0)
{
}

template <ComputeStage compute_stage>
MooseEnum
ADViscoplasticityStressUpdate<compute_stage>::getModelEnum()
{
  return MooseEnum("LPS GTN", "LPS");
}

template <ComputeStage compute_stage>
MooseEnum
ADViscoplasticityStressUpdate<compute_stage>::getPoreShapeEnum()
{
  return MooseEnum("spherical cylindrical", "spherical");
}

template <ComputeStage compute_stage>
void
ADViscoplasticityStressUpdate<compute_stage>::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
  _creep_strain[_qp].zero();
}

template <ComputeStage compute_stage>
void
ADViscoplasticityStressUpdate<compute_stage>::propagateQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _creep_strain[_qp] = _creep_strain_old[_qp];
}

template <ComputeStage compute_stage>
void
ADViscoplasticityStressUpdate<compute_stage>::updateState(
    ADRankTwoTensor & elastic_strain_increment,
    ADRankTwoTensor & inelastic_strain_increment,
    const ADRankTwoTensor & /*rotation_increment*/,
    ADRankTwoTensor & stress,
    const RankTwoTensor & /*stress_old*/,
    const ADRankFourTensor & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old)
{
  // Compute initial hydrostatic stress and porosity
  if (_pore_shape == "cylindrical")
    _hydro_stress = (stress(0, 0) + stress(1, 1)) / 2.0;
  else
    _hydro_stress = stress.trace() / 3.0;

  // Subtract elastic strain from strain increment to find all inelastic strain increments
  // calculated so far except the one that we're about to calculate
  const ADRankTwoTensor inelastic_volumetric_increment =
      _strain_increment[_qp] - elastic_strain_increment;
  // Calculate intermdiate porosity from all inelastic strain increments calculated so far except
  // the one that we're about to calculate
  _intermediate_porosity =
      (1.0 - _porosity_old[_qp]) * inelastic_volumetric_increment.trace() + _porosity_old[_qp];

  // Compute intermediate equivalent stress
  const ADRankTwoTensor dev_stress = stress.deviatoric();
  const ADReal dev_stress_squared = dev_stress.doubleContraction(dev_stress);
  const ADReal equiv_stress = MooseUtils::absoluteFuzzyEqual(dev_stress_squared, 0.0)
                                  ? 0.0
                                  : std::sqrt(1.5 * dev_stress_squared);

  computeStressInitialize(equiv_stress, elasticity_tensor);

  // Prepare values
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _creep_strain[_qp] = _creep_strain_old[_qp];
  inelastic_strain_increment.zero();

  // If equivalent stress is present, calculate creep strain increment
  if (equiv_stress)
  {
    // Initalize stress potential
    ADReal dpsi_dgauge(0);
    computeInelasticStrainIncrement(_gauge_stress[_qp],
                                    dpsi_dgauge,
                                    inelastic_strain_increment,
                                    equiv_stress,
                                    dev_stress,
                                    stress);

    // Update elastic strain increment due to inelastic strain calculated here
    elastic_strain_increment -= inelastic_strain_increment;
    // Update stress due to new strain
    stress = elasticity_tensor * (elastic_strain_old + elastic_strain_increment);
    // Compute effective strain from the stress potential
    _effective_inelastic_strain[_qp] += dpsi_dgauge * _dt;
    // Update creep strain due to currently computed inelastic strain
    _creep_strain[_qp] += inelastic_strain_increment;
  }

  computeStressFinalize(inelastic_strain_increment);
}

template <ComputeStage compute_stage>
Real
ADViscoplasticityStressUpdate<compute_stage>::computeReferenceResidual(
    const ADReal & /*effective_trial_stress*/, const ADReal & gauge_stress)
{
  // Use gauge stress for relative tolerance criteria, defined as:
  // std::abs(residual / gauge_stress) <= _relative_tolerance
  return MetaPhysicL::raw_value(gauge_stress);
}

template <ComputeStage compute_stage>
ADReal
ADViscoplasticityStressUpdate<compute_stage>::initialGuess(const ADReal & effective_trial_stress)
{
  return effective_trial_stress;
}

template <ComputeStage compute_stage>
ADReal
ADViscoplasticityStressUpdate<compute_stage>::maximumPermissibleValue(
    const ADReal & effective_trial_stress) const
{
  return effective_trial_stress * 1.0e6;
}

template <ComputeStage compute_stage>
ADReal
ADViscoplasticityStressUpdate<compute_stage>::minimumPermissibleValue(
    const ADReal & effective_trial_stress) const
{
  return effective_trial_stress;
}

template <ComputeStage compute_stage>
Real
ADViscoplasticityStressUpdate<compute_stage>::computeTimeStepLimit()
{
  const Real scalar_inelastic_strain_incr =
      MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
      _effective_inelastic_strain_old[_qp];

  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}

template <ComputeStage compute_stage>
void
ADViscoplasticityStressUpdate<compute_stage>::outputIterationSummary(
    std::stringstream * iter_output, const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  ADSingleVariableReturnMappingSolution<compute_stage>::outputIterationSummary(iter_output,
                                                                               total_it);
}

template <ComputeStage compute_stage>
ADReal
ADViscoplasticityStressUpdate<compute_stage>::computeResidual(const ADReal & equiv_stress,
                                                              const ADReal & trial_gauge)
{
  const ADReal M = std::abs(_hydro_stress) / trial_gauge;
  const ADReal dM_dtrial_gauge = -M / trial_gauge;

  const ADReal residual_left = Utility::pow<2>(equiv_stress / trial_gauge);
  const ADReal dresidual_left_dtrial_gauge = -2.0 * residual_left / trial_gauge;

  ADReal residual = residual_left;
  _derivative = dresidual_left_dtrial_gauge;

  if (_pore_shape == "spherical")
  {
    residual *= 1.0 + _intermediate_porosity / 1.5;
    _derivative *= 1.0 + _intermediate_porosity / 1.5;
  }

  if (_model == "GTN")
  {
    residual += 2.0 * _intermediate_porosity * std::cosh(_pore_shape_factor * M) - 1.0 -
                Utility::pow<2>(_intermediate_porosity);
    _derivative += 2.0 * _intermediate_porosity * std::sinh(_pore_shape_factor * M) *
                   _pore_shape_factor * dM_dtrial_gauge;
  }
  else
  {
    const ADReal h = computeH(_power, M);
    const ADReal dh_dM = computeH(_power, M, true);

    residual += _intermediate_porosity * (h + _power_factor / h) - 1.0 -
                _power_factor * Utility::pow<2>(_intermediate_porosity);
    const ADReal dresidual_dh = _intermediate_porosity * (1.0 - _power_factor / Utility::pow<2>(h));
    _derivative += dresidual_dh * dh_dM * dM_dtrial_gauge;
  }

  if (_verbose)
  {
    Moose::out << "in computeResidual:\n"
               << "  position: " << _q_point[_qp] << " hydro_stress: " << _hydro_stress
               << " equiv_stress: " << equiv_stress << " trial_grage: " << trial_gauge
               << " M: " << M << std::endl;
    Moose::out << "  residual: " << residual << "  derivative: " << _derivative << std::endl;
  }

  return residual;
}

template <ComputeStage compute_stage>
ADReal
ADViscoplasticityStressUpdate<compute_stage>::computeH(const Real n,
                                                       const ADReal & M,
                                                       const bool derivative)
{
  const ADReal mod = std::pow(M * _pore_shape_factor, (n + 1.0) / n);

  // Calculate derivative with respect to M
  if (derivative)
  {
    const ADReal dmod_dM = (n + 1.0) / n * mod / M;
    return dmod_dM * std::pow(1.0 + mod / n, n - 1.0);
  }

  return std::pow(1.0 + mod / n, n);
}

template <ComputeStage compute_stage>
ADRankTwoTensor
ADViscoplasticityStressUpdate<compute_stage>::computeDGaugeDSigma(
    const ADReal & gauge_stress,
    const ADReal & equiv_stress,
    const ADRankTwoTensor & dev_stress,
    const ADRankTwoTensor & stress)
{
  // Compute the derivative of the gauge stress with respect to the equilvalent and hydrostatic
  // stress components
  const ADReal M = std::abs(_hydro_stress) / gauge_stress;
  const ADReal h = computeH(_power, M);

  // Compute the derviative of the residual with respect to the hydrostatic stress
  ADReal dresidual_dhydro_stress = 0.0;
  if (_hydro_stress)
  {
    const ADReal dM_dhydro_stress = M / _hydro_stress;
    if (_model == "GTN")
    {
      dresidual_dhydro_stress = 2.0 * _intermediate_porosity * std::sinh(_pore_shape_factor * M) *
                                _pore_shape_factor * dM_dhydro_stress;
    }
    else
    {
      const ADReal dresidual_dh =
          _intermediate_porosity * (1.0 - _power_factor / Utility::pow<2>(h));
      const ADReal dh_dM = computeH(_power, M, true);
      dresidual_dhydro_stress = dresidual_dh * dh_dM * dM_dhydro_stress;
    }
  }

  // Compute the derivative of the residual with respect to the equilvalent stress
  ADReal dresidual_dequiv_stress = 2.0 * equiv_stress / Utility::pow<2>(gauge_stress);
  if (_pore_shape == "spherical")
    dresidual_dequiv_stress *= 1.0 + _intermediate_porosity / 1.5;

  // Compute the derivative of the equilvalent stress to the deviatoric stress
  const ADRankTwoTensor dequiv_stress_dsigma = 1.5 * dev_stress / equiv_stress;

  // Compute the derivative of the residual with the stress
  const ADRankTwoTensor dresidual_dsigma = dresidual_dhydro_stress * _dhydro_stress_dsigma +
                                           dresidual_dequiv_stress * dequiv_stress_dsigma;

  // Compute the deritative of the gauge stress with respect to the stress
  const ADRankTwoTensor dgauge_dsigma =
      gauge_stress * dresidual_dsigma / dresidual_dsigma.doubleContraction(stress);

  return dgauge_dsigma;
}

template <ComputeStage compute_stage>
void
ADViscoplasticityStressUpdate<compute_stage>::computeInelasticStrainIncrement(
    ADReal & gauge_stress,
    ADReal & dpsi_dgauge,
    ADRankTwoTensor & inelastic_strain_increment,
    const ADReal & equiv_stress,
    const ADRankTwoTensor & dev_stress,
    const ADRankTwoTensor & stress)
{
  // If hydrostatic stress and porosity present, compute non-linear gauge stress
  if (!_intermediate_porosity)
    gauge_stress = equiv_stress;
  else if (_hydro_stress)
    returnMappingSolve(equiv_stress, gauge_stress, _console);
  else
    gauge_stress /= std::sqrt(1.0 + _power_factor * Utility::pow<2>(_intermediate_porosity));

    mooseAssert(gauge_stress > equiv_stress,
                "Gauge stress calculated in inner Newton solve is less than equilvalent.");

  // Compute stress potential
  dpsi_dgauge = _coefficient[_qp] * std::pow(gauge_stress, _power);

  // Compute strain increment from stress potential and the gauge stress derivative with respect
  // to the stress stress. The current form is explicit, and should eventually be changed
  inelastic_strain_increment +=
      _dt * dpsi_dgauge * computeDGaugeDSigma(gauge_stress, equiv_stress, dev_stress, stress);
}
