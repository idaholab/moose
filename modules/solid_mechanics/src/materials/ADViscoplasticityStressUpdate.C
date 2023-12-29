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

registerMooseObject("TensorMechanicsApp", ADViscoplasticityStressUpdate);

InputParameters
ADViscoplasticityStressUpdate::validParams()
{
  InputParameters params = ADViscoplasticityStressUpdateBase::validParams();
  params += ADSingleVariableReturnMappingSolution::validParams();
  params.addClassDescription(
      "This material computes the non-linear homogenized gauge stress in order to compute the "
      "viscoplastic responce due to creep in porous materials. This material must be used in "
      "conjunction with ADComputeMultiplePorousInelasticStress");
  MooseEnum viscoplasticity_model("LPS GTN", "LPS");
  params.addParam<MooseEnum>(
      "viscoplasticity_model", viscoplasticity_model, "Which viscoplastic model to use");
  MooseEnum pore_shape_model("spherical cylindrical", "spherical");
  params.addParam<MooseEnum>("pore_shape_model", pore_shape_model, "Which pore shape model to use");
  params.addRequiredParam<MaterialPropertyName>(
      "coefficient", "Material property name for the leading coefficient for Norton power law");
  params.addRequiredRangeCheckedParam<Real>(
      "power", "power>=1.0", "Stress exponent for Norton power law");
  params.addParam<Real>(
      "maximum_gauge_ratio",
      1.0e6,
      "Maximum ratio between the gauge stress and the equivalent stress. This "
      "should be a high number. Note that this does not set an upper bound on the value, but "
      "rather will help with convergence of the inner Newton loop");
  params.addParam<Real>(
      "minimum_equivalent_stress",
      1.0e-3,
      "Minimum value of equivalent stress below which viscoplasticiy is not calculated.");
  params.addParam<Real>("maximum_equivalent_stress",
                        1.0e12,
                        "Maximum value of equivalent stress above which an exception is thrown "
                        "instead of calculating the properties in this material.");

  params.addParamNamesToGroup("verbose maximum_gauge_ratio maximum_equivalent_stress", "Advanced");
  return params;
}

ADViscoplasticityStressUpdate::ADViscoplasticityStressUpdate(const InputParameters & parameters)
  : ADViscoplasticityStressUpdateBase(parameters),
    ADSingleVariableReturnMappingSolution(parameters),
    _model(parameters.get<MooseEnum>("viscoplasticity_model").getEnum<ViscoplasticityModel>()),
    _pore_shape(parameters.get<MooseEnum>("pore_shape_model").getEnum<PoreShapeModel>()),
    _pore_shape_factor(_pore_shape == PoreShapeModel::SPHERICAL ? 1.5 : std::sqrt(3.0)),
    _power(getParam<Real>("power")),
    _power_factor(_model == ViscoplasticityModel::LPS ? (_power - 1.0) / (_power + 1.0) : 1.0),
    _coefficient(getADMaterialProperty<Real>("coefficient")),
    _gauge_stress(declareADProperty<Real>(_base_name + "gauge_stress")),
    _maximum_gauge_ratio(getParam<Real>("maximum_gauge_ratio")),
    _minimum_equivalent_stress(getParam<Real>("minimum_equivalent_stress")),
    _maximum_equivalent_stress(getParam<Real>("maximum_equivalent_stress")),
    _hydro_stress(0.0),
    _identity_two(RankTwoTensor::initIdentity),
    _dhydro_stress_dsigma(_identity_two / 3.0),
    _derivative(0.0)
{
  _check_range = true;
}

void
ADViscoplasticityStressUpdate::updateState(ADRankTwoTensor & elastic_strain_increment,
                                           ADRankTwoTensor & inelastic_strain_increment,
                                           const ADRankTwoTensor & /*rotation_increment*/,
                                           ADRankTwoTensor & stress,
                                           const RankTwoTensor & /*stress_old*/,
                                           const ADRankFourTensor & elasticity_tensor,
                                           const RankTwoTensor & elastic_strain_old,
                                           bool /*compute_full_tangent_operator = false*/,
                                           RankFourTensor & /*tangent_operator = _identityTensor*/)
{
  // Compute initial hydrostatic stress and porosity
  if (_pore_shape == PoreShapeModel::CYLINDRICAL)
    _hydro_stress = (stress(0, 0) + stress(1, 1)) / 2.0;
  else
    _hydro_stress = stress.trace() / 3.0;

  updateIntermediatePorosity(elastic_strain_increment);

  // Compute intermediate equivalent stress
  const ADRankTwoTensor dev_stress = stress.deviatoric();
  const ADReal dev_stress_squared = dev_stress.doubleContraction(dev_stress);
  const ADReal equiv_stress = dev_stress_squared == 0.0 ? 0.0 : std::sqrt(1.5 * dev_stress_squared);

  computeStressInitialize(equiv_stress, elasticity_tensor);

  // Prepare values
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp];
  inelastic_strain_increment.zero();

  // Protect against extremely high values of stresses calculated by other viscoplastic materials
  if (equiv_stress > _maximum_equivalent_stress)
    mooseException("In ",
                   _name,
                   ": equivalent stress (",
                   equiv_stress,
                   ") is higher than maximum_equivalent_stress (",
                   _maximum_equivalent_stress,
                   ").\nCutting time step.");

  // If equivalent stress is present, calculate creep strain increment
  if (equiv_stress > _minimum_equivalent_stress)
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

    // Compute effective strain from the stress potential. Note that this is approximate and to be
    // used qualitatively
    _effective_inelastic_strain[_qp] += dpsi_dgauge * _dt;
    // Update creep strain due to currently computed inelastic strain
    _inelastic_strain[_qp] += inelastic_strain_increment;
  }

  const ADRankTwoTensor new_dev_stress = stress.deviatoric();
  const ADReal new_dev_stress_squared = new_dev_stress.doubleContraction(new_dev_stress);
  const ADReal new_equiv_stress =
      new_dev_stress_squared == 0.0 ? 0.0 : std::sqrt(1.5 * new_dev_stress_squared);

  if (MooseUtils::relativeFuzzyGreaterThan(new_equiv_stress, equiv_stress))
    mooseException("In ",
                   _name,
                   ": updated equivalent stress (",
                   MetaPhysicL::raw_value(new_equiv_stress),
                   ") is greater than initial equivalent stress (",
                   MetaPhysicL::raw_value(equiv_stress),
                   "). Try decreasing max_inelastic_increment to avoid this exception.");

  computeStressFinalize(inelastic_strain_increment);
}

ADReal
ADViscoplasticityStressUpdate::initialGuess(const ADReal & effective_trial_stress)
{
  return effective_trial_stress;
}

ADReal
ADViscoplasticityStressUpdate::maximumPermissibleValue(const ADReal & effective_trial_stress) const
{
  return effective_trial_stress * _maximum_gauge_ratio;
}

ADReal
ADViscoplasticityStressUpdate::minimumPermissibleValue(const ADReal & effective_trial_stress) const
{
  return effective_trial_stress;
}

ADReal
ADViscoplasticityStressUpdate::computeResidual(const ADReal & equiv_stress,
                                               const ADReal & trial_gauge)
{
  const ADReal M = std::abs(_hydro_stress) / trial_gauge;
  const ADReal dM_dtrial_gauge = -M / trial_gauge;

  const ADReal residual_left = Utility::pow<2>(equiv_stress / trial_gauge);
  const ADReal dresidual_left_dtrial_gauge = -2.0 * residual_left / trial_gauge;

  ADReal residual = residual_left;
  _derivative = dresidual_left_dtrial_gauge;

  if (_pore_shape == PoreShapeModel::SPHERICAL)
  {
    residual *= 1.0 + _intermediate_porosity / 1.5;
    _derivative *= 1.0 + _intermediate_porosity / 1.5;
  }

  if (_model == ViscoplasticityModel::GTN)
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

ADReal
ADViscoplasticityStressUpdate::computeH(const Real n, const ADReal & M, const bool derivative)
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

ADRankTwoTensor
ADViscoplasticityStressUpdate::computeDGaugeDSigma(const ADReal & gauge_stress,
                                                   const ADReal & equiv_stress,
                                                   const ADRankTwoTensor & dev_stress,
                                                   const ADRankTwoTensor & stress)
{
  // Compute the derivative of the gauge stress with respect to the equivalent and hydrostatic
  // stress components
  const ADReal M = std::abs(_hydro_stress) / gauge_stress;
  const ADReal h = computeH(_power, M);

  // Compute the derviative of the residual with respect to the hydrostatic stress
  ADReal dresidual_dhydro_stress = 0.0;
  if (_hydro_stress != 0.0)
  {
    const ADReal dM_dhydro_stress = M / _hydro_stress;
    if (_model == ViscoplasticityModel::GTN)
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

  // Compute the derivative of the residual with respect to the equivalent stress
  ADReal dresidual_dequiv_stress = 2.0 * equiv_stress / Utility::pow<2>(gauge_stress);
  if (_pore_shape == PoreShapeModel::SPHERICAL)
    dresidual_dequiv_stress *= 1.0 + _intermediate_porosity / 1.5;

  // Compute the derivative of the equilvalent stress to the deviatoric stress
  const ADRankTwoTensor dequiv_stress_dsigma = 1.5 * dev_stress / equiv_stress;

  // Compute the derivative of the residual with the stress
  const ADRankTwoTensor dresidual_dsigma = dresidual_dhydro_stress * _dhydro_stress_dsigma +
                                           dresidual_dequiv_stress * dequiv_stress_dsigma;

  // Compute the deritative of the gauge stress with respect to the stress
  const ADRankTwoTensor dgauge_dsigma =
      dresidual_dsigma * (gauge_stress / dresidual_dsigma.doubleContraction(stress));

  return dgauge_dsigma;
}

void
ADViscoplasticityStressUpdate::computeInelasticStrainIncrement(
    ADReal & gauge_stress,
    ADReal & dpsi_dgauge,
    ADRankTwoTensor & inelastic_strain_increment,
    const ADReal & equiv_stress,
    const ADRankTwoTensor & dev_stress,
    const ADRankTwoTensor & stress)
{
  // If hydrostatic stress and porosity present, compute non-linear gauge stress
  if (_intermediate_porosity == 0.0)
    gauge_stress = equiv_stress;
  else if (_hydro_stress == 0.0)
    gauge_stress =
        equiv_stress / std::sqrt(1.0 - (1.0 + _power_factor) * _intermediate_porosity +
                                 _power_factor * Utility::pow<2>(_intermediate_porosity));
  else
    returnMappingSolve(equiv_stress, gauge_stress, _console);

  mooseAssert(gauge_stress >= equiv_stress,
              "Gauge stress calculated in inner Newton solve is less than the equivalent stress.");

  // Compute stress potential
  dpsi_dgauge = _coefficient[_qp] * std::pow(gauge_stress, _power);

  // Compute strain increment from stress potential and the gauge stress derivative with respect
  // to the stress stress. The current form is explicit, and should eventually be changed
  inelastic_strain_increment =
      _dt * dpsi_dgauge * computeDGaugeDSigma(gauge_stress, equiv_stress, dev_stress, stress);
}

void
ADViscoplasticityStressUpdate::outputIterationSummary(std::stringstream * iter_output,
                                                      const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  ADSingleVariableReturnMappingSolution::outputIterationSummary(iter_output, total_it);
}

Real
ADViscoplasticityStressUpdate::computeReferenceResidual(const ADReal & /*effective_trial_stress*/,
                                                        const ADReal & gauge_stress)
{
  // Use gauge stress for relative tolerance criteria, defined as:
  // std::abs(residual / gauge_stress) <= _relative_tolerance
  return MetaPhysicL::raw_value(gauge_stress);
}
