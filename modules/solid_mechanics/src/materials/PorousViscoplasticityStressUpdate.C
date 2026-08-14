//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADViscoplasticityStressUpdate.h"

#include "libmesh/utility.h"

registerMooseObject("SolidMechanicsApp", ADViscoplasticityStressUpdate);

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

  MooseEnum substepping_type("NONE INCREMENT_BASED", "NONE");
  substepping_type.addDocumentation("NONE", "Do not use local constitutive substepping");
  substepping_type.addDocumentation(
      "INCREMENT_BASED",
      "Use explicit local substeps sized from the predicted viscoplastic strain increment.");
  params.addParam<MooseEnum>(
      "use_substepping", substepping_type, "Whether and how to use local constitutive substepping");
  params.addRangeCheckedParam<Real>(
      "substep_strain_tolerance",
      0.1,
      "substep_strain_tolerance>0.0",
      "Target fraction of max_inelastic_increment allowed in one local substep. Reduce this "
      "value to increase the number of substeps.");
  params.addParam<bool>(
      "adaptive_substepping",
      false,
      "If a local constitutive update fails, successively double the number of substeps.");
  params.addParam<unsigned int>(
      "maximum_number_substeps",
      25,
      "Maximum number of local constitutive substeps before cutting the global timestep.");

  params.addParamNamesToGroup(
      "verbose maximum_gauge_ratio maximum_equivalent_stress use_substepping "
      "substep_strain_tolerance adaptive_substepping maximum_number_substeps",
      "Advanced");
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
    _use_substepping(getParam<MooseEnum>("use_substepping").getEnum<SubsteppingType>()),
    _substep_tolerance(getParam<Real>("substep_strain_tolerance")),
    _adaptive_substepping(getParam<bool>("adaptive_substepping")),
    _maximum_number_substeps(getParam<unsigned int>("maximum_number_substeps")),
    _dt_original(0.0),
    _hydro_stress(0.0),
    _identity_two(RankTwoTensor::initIdentity),
    _dhydro_stress_dsigma(_identity_two / 3.0),
    _derivative(0.0)
{
  _check_range = true;

  if (_pars.isParamSetByUser("maximum_number_substeps") &&
      _use_substepping == SubsteppingType::NONE)
    paramError("maximum_number_substeps",
               "maximum_number_substeps can only be used when use_substepping is enabled.");

  if (_adaptive_substepping && _use_substepping == SubsteppingType::NONE)
    paramError("adaptive_substepping",
               "adaptive_substepping can only be used when use_substepping is enabled.");
}

bool
ADViscoplasticityStressUpdate::substeppingCapabilityEnabled()
{
  return _use_substepping != SubsteppingType::NONE;
}

bool
ADViscoplasticityStressUpdate::substeppingCapabilityRequested()
{
  return _use_substepping != SubsteppingType::NONE;
}

void
ADViscoplasticityStressUpdate::resetIncrementalMaterialProperties()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp];
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
  updateIntermediatePorosity(elastic_strain_increment);

  resetIncrementalMaterialProperties();
  inelastic_strain_increment.zero();

  const ADRankTwoTensor elastic_strain_old_ad = elastic_strain_old;
  ADReal effective_inelastic_strain_increment = 0.0;

  updateStateOneStep(elastic_strain_increment,
                     inelastic_strain_increment,
                     stress,
                     elasticity_tensor,
                     elastic_strain_old_ad,
                     effective_inelastic_strain_increment);

  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + effective_inelastic_strain_increment;
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp] + inelastic_strain_increment;

  computeStressFinalize(inelastic_strain_increment);
}

void
ADViscoplasticityStressUpdate::updateStateOneStep(ADRankTwoTensor & elastic_strain_increment,
                                                  ADRankTwoTensor & inelastic_strain_increment,
                                                  ADRankTwoTensor & stress,
                                                  const ADRankFourTensor & elasticity_tensor,
                                                  const ADRankTwoTensor & elastic_strain_old,
                                                  ADReal & effective_inelastic_strain_increment)
{
  using std::sqrt;

  // Compute initial hydrostatic stress and porosity
  if (_pore_shape == PoreShapeModel::CYLINDRICAL)
    _hydro_stress = (stress(0, 0) + stress(1, 1)) / 2.0;
  else
    _hydro_stress = stress.trace() / 3.0;

  // Compute intermediate equivalent stress
  const auto dev_stress = stress.deviatoric();
  const auto dev_stress_squared = dev_stress.doubleContraction(dev_stress);
  const auto equiv_stress = dev_stress_squared == 0.0 ? 0.0 : sqrt(1.5 * dev_stress_squared);

  computeStressInitialize(equiv_stress, elasticity_tensor);

  // Prepare values
  inelastic_strain_increment.zero();
  effective_inelastic_strain_increment = 0.0;

  // Protect against extremely high values of stresses calculated by other viscoplastic materials
  if (equiv_stress > _maximum_equivalent_stress)
    mooseException("In ",
                   _name,
                   ": equivalent stress (",
                   MetaPhysicL::raw_value(equiv_stress),
                   ") is higher than maximum_equivalent_stress (",
                   _maximum_equivalent_stress,
                   ").\nCutting time step.");

  // If equivalent stress is present, calculate creep strain increment
  if (equiv_stress > _minimum_equivalent_stress)
  {
    // Initalize stress potential
    ADReal dpsi_dgauge = 0.0;

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

    effective_inelastic_strain_increment = dpsi_dgauge * _dt;
  }

  const auto new_dev_stress = stress.deviatoric();
  const auto new_dev_stress_squared = new_dev_stress.doubleContraction(new_dev_stress);
  const auto new_equiv_stress =
      new_dev_stress_squared == 0.0 ? 0.0 : sqrt(1.5 * new_dev_stress_squared);

  if (MooseUtils::relativeFuzzyGreaterThan(new_equiv_stress, equiv_stress))
    mooseException("In ",
                   _name,
                   ": updated equivalent stress (",
                   MetaPhysicL::raw_value(new_equiv_stress),
                   ") is greater than initial equivalent stress (",
                   MetaPhysicL::raw_value(equiv_stress),
                   "). Increase the number of local substeps or decrease the global time step.");
}

unsigned int
ADViscoplasticityStressUpdate::estimateNumberSubsteps(const ADRankTwoTensor & stress)
{
  using std::ceil;
  using std::pow;
  using std::sqrt;

  if (_pore_shape == PoreShapeModel::CYLINDRICAL)
    _hydro_stress = (stress(0, 0) + stress(1, 1)) / 2.0;
  else
    _hydro_stress = stress.trace() / 3.0;

  const auto dev_stress = stress.deviatoric();
  const auto dev_stress_squared = dev_stress.doubleContraction(dev_stress);
  const auto equiv_stress = dev_stress_squared == 0.0 ? 0.0 : sqrt(1.5 * dev_stress_squared);

  if (equiv_stress <= _minimum_equivalent_stress)
    return 1;

  if (equiv_stress > _maximum_equivalent_stress)
    mooseException("In ",
                   _name,
                   ": equivalent stress is higher than maximum_equivalent_stress while "
                   "estimating local substeps.");

  auto gauge_stress = equiv_stress;
  computeGaugeStress(gauge_stress, equiv_stress);

  const auto dpsi_dgauge = _coefficient[_qp] * pow(gauge_stress, _power);
  const auto estimated_effective_increment = std::abs(MetaPhysicL::raw_value(dpsi_dgauge)) * _dt;
  const auto target_increment = _substep_tolerance * _max_inelastic_increment;

  if (estimated_effective_increment <= target_increment)
    return 1;

  return static_cast<unsigned int>(ceil(estimated_effective_increment / target_increment));
}

void
ADViscoplasticityStressUpdate::updateStateSubstepInternal(
    ADRankTwoTensor & strain_increment,
    ADRankTwoTensor & inelastic_strain_increment,
    const ADRankTwoTensor & rotation_increment,
    ADRankTwoTensor & stress_new,
    const RankTwoTensor & stress_old,
    const ADRankFourTensor & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    unsigned int total_number_substeps,
    bool compute_full_tangent_operator,
    RankFourTensor & tangent_operator)
{
  if (total_number_substeps == 0)
    mooseError("ADViscoplasticityStressUpdate received zero substeps.");

  if (total_number_substeps == 1)
  {
    updateState(strain_increment,
                inelastic_strain_increment,
                rotation_increment,
                stress_new,
                stress_old,
                elasticity_tensor,
                elastic_strain_old,
                compute_full_tangent_operator,
                tangent_operator);
    return;
  }

  if (total_number_substeps > _maximum_number_substeps)
    mooseException("The number of substeps computed exceeds 'maximum_number_substeps'.");

  _dt = _dt_original / total_number_substeps;

  const auto strain_increment_per_step =
      strain_increment / static_cast<Real>(total_number_substeps);

  ADRankTwoTensor sub_elastic_strain_old = elastic_strain_old;
  auto sub_stress_new = elasticity_tensor * sub_elastic_strain_old;

  strain_increment.zero();
  inelastic_strain_increment.zero();
  stress_new.zero();

  ADReal accumulated_effective_inelastic_strain_increment = 0.0;

  for (unsigned int step = 0; step < total_number_substeps; ++step)
  {
    auto sub_strain_increment = strain_increment_per_step;
    ADRankTwoTensor sub_inelastic_strain_increment;
    sub_inelastic_strain_increment.zero();

    sub_stress_new += elasticity_tensor * sub_strain_increment;

    ADReal sub_effective_inelastic_strain_increment = 0.0;
    updateStateOneStep(sub_strain_increment,
                       sub_inelastic_strain_increment,
                       sub_stress_new,
                       elasticity_tensor,
                       sub_elastic_strain_old,
                       sub_effective_inelastic_strain_increment);

    strain_increment += sub_strain_increment;
    inelastic_strain_increment += sub_inelastic_strain_increment;
    sub_elastic_strain_old += sub_strain_increment;

    sub_stress_new = elasticity_tensor * sub_elastic_strain_old;
    accumulated_effective_inelastic_strain_increment += sub_effective_inelastic_strain_increment;

    if (_verbose)
      Moose::out << "ADViscoplasticityStressUpdate substep " << step + 1 << "/"
                 << total_number_substeps << " dt_sub = " << _dt
                 << " effective inelastic increment = "
                 << MetaPhysicL::raw_value(sub_effective_inelastic_strain_increment) << std::endl;
  }

  stress_new = sub_stress_new;
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + accumulated_effective_inelastic_strain_increment;
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp] + inelastic_strain_increment;

  computeStressFinalize(inelastic_strain_increment);
}

void
ADViscoplasticityStressUpdate::updateStateSubstep(ADRankTwoTensor & strain_increment,
                                                  ADRankTwoTensor & inelastic_strain_increment,
                                                  const ADRankTwoTensor & rotation_increment,
                                                  ADRankTwoTensor & stress_new,
                                                  const RankTwoTensor & stress_old,
                                                  const ADRankFourTensor & elasticity_tensor,
                                                  const RankTwoTensor & elastic_strain_old,
                                                  bool compute_full_tangent_operator,
                                                  RankFourTensor & tangent_operator)
{
  _dt_original = _dt;

  const auto original_strain_increment = strain_increment;
  const auto original_stress_new = stress_new;
  const auto original_gauge_stress = _gauge_stress[_qp];

  // Keep this model's intermediate porosity fixed during the local substeps. It still includes
  // porosity associated with inelastic increments already computed by the other inelastic models.
  updateIntermediatePorosity(original_strain_increment);

  unsigned int number_substeps = 1;
  try
  {
    number_substeps = estimateNumberSubsteps(original_stress_new);
  }
  catch (MooseException &)
  {
    _dt = _dt_original;
    if (!_adaptive_substepping)
      throw;
    number_substeps = 2;
  }

  while (true)
  {
    if (number_substeps > _maximum_number_substeps)
    {
      _dt = _dt_original;
      mooseException("In ",
                     _name,
                     ": required number of viscoplastic substeps (",
                     number_substeps,
                     ") exceeds maximum_number_substeps (",
                     _maximum_number_substeps,
                     "). Cutting global time step.");
    }

    strain_increment = original_strain_increment;
    inelastic_strain_increment.zero();
    stress_new = original_stress_new;
    _gauge_stress[_qp] = original_gauge_stress;
    resetIncrementalMaterialProperties();
    updateIntermediatePorosity(original_strain_increment);

    try
    {
      updateStateSubstepInternal(strain_increment,
                                 inelastic_strain_increment,
                                 rotation_increment,
                                 stress_new,
                                 stress_old,
                                 elasticity_tensor,
                                 elastic_strain_old,
                                 number_substeps,
                                 compute_full_tangent_operator,
                                 tangent_operator);

      _dt = _dt_original;
      return;
    }
    catch (MooseException &)
    {
      _dt = _dt_original;

      if (!_adaptive_substepping)
        throw;

      if (number_substeps >= _maximum_number_substeps)
        break;

      number_substeps = number_substeps > _maximum_number_substeps / 2 ? _maximum_number_substeps
                                                                       : 2 * number_substeps;
    }
  }

  _dt = _dt_original;
  mooseException("In ",
                 _name,
                 ": adaptive viscoplastic substepping failed after reaching "
                 "maximum_number_substeps = ",
                 _maximum_number_substeps,
                 ". Cutting global time step.");
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
  using std::abs, std::cosh, std::sinh;

  const auto M = abs(_hydro_stress) / trial_gauge;
  const auto dM_dtrial_gauge = -M / trial_gauge;

  const auto residual_left = Utility::pow<2>(equiv_stress / trial_gauge);
  const auto dresidual_left_dtrial_gauge = -2.0 * residual_left / trial_gauge;

  auto residual = residual_left;
  _derivative = dresidual_left_dtrial_gauge;

  if (_pore_shape == PoreShapeModel::SPHERICAL)
  {
    residual *= 1.0 + _intermediate_porosity / 1.5;
    _derivative *= 1.0 + _intermediate_porosity / 1.5;
  }

  if (_model == ViscoplasticityModel::GTN)
  {
    residual += 2.0 * _intermediate_porosity * cosh(_pore_shape_factor * M) - 1.0 -
                Utility::pow<2>(_intermediate_porosity);
    _derivative += 2.0 * _intermediate_porosity * sinh(_pore_shape_factor * M) *
                   _pore_shape_factor * dM_dtrial_gauge;
  }
  else
  {
    const auto h = computeH(_power, M);
    const auto dh_dM = computeH(_power, M, true);

    residual += _intermediate_porosity * (h + _power_factor / h) - 1.0 -
                _power_factor * Utility::pow<2>(_intermediate_porosity);
    const auto dresidual_dh = _intermediate_porosity * (1.0 - _power_factor / Utility::pow<2>(h));
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
  using std::pow;

  const auto mod = pow(M * _pore_shape_factor, (n + 1.0) / n);

  // Calculate derivative with respect to M
  if (derivative)
  {
    const auto dmod_dM = (n + 1.0) / n * mod / M;
    return dmod_dM * pow(1.0 + mod / n, n - 1.0);
  }

  return pow(1.0 + mod / n, n);
}

ADRankTwoTensor
ADViscoplasticityStressUpdate::computeDGaugeDSigma(const ADReal & gauge_stress,
                                                   const ADReal & /*equiv_stress*/,
                                                   const ADRankTwoTensor & dev_stress,
                                                   const ADRankTwoTensor & stress)
{
  using std::abs, std::sinh;

  // Compute the derivative of the gauge stress with respect to the equivalent and hydrostatic
  // stress components
  const auto M = abs(_hydro_stress) / gauge_stress;
  const auto h = computeH(_power, M);

  // Compute the derviative of the residual with respect to the hydrostatic stress
  ADReal dresidual_dhydro_stress = 0.0;
  if (_hydro_stress != 0.0)
  {
    const auto dM_dhydro_stress = M / _hydro_stress;
    if (_model == ViscoplasticityModel::GTN)
    {
      dresidual_dhydro_stress = 2.0 * _intermediate_porosity * sinh(_pore_shape_factor * M) *
                                _pore_shape_factor * dM_dhydro_stress;
    }
    else
    {
      const auto dresidual_dh = _intermediate_porosity * (1.0 - _power_factor / Utility::pow<2>(h));
      const auto dh_dM = computeH(_power, M, true);
      dresidual_dhydro_stress = dresidual_dh * dh_dM * dM_dhydro_stress;
    }
  }

  // Combine dresidual_dequiv_stress * dequiv_stress_dsigma to cancel out equiv_stress to avoid
  // nan's when equiv_stress=0

  auto dresidual_dequiv_stress_dequiv_stress_dsigma =
      3.0 * dev_stress / Utility::pow<2>(gauge_stress);
  if (_pore_shape == PoreShapeModel::SPHERICAL)
    dresidual_dequiv_stress_dequiv_stress_dsigma *= 1.0 + _intermediate_porosity / 1.5;

  // Compute the derivative of the residual with the stress
  const ADRankTwoTensor dresidual_dsigma = dresidual_dhydro_stress * _dhydro_stress_dsigma +
                                           dresidual_dequiv_stress_dequiv_stress_dsigma;

  // Compute the deritative of the gauge stress with respect to the stress
  const auto dgauge_dsigma =
      dresidual_dsigma * (gauge_stress / dresidual_dsigma.doubleContraction(stress));

  return dgauge_dsigma;
}

void
ADViscoplasticityStressUpdate::computeGaugeStress(ADReal & gauge_stress,
                                                  const ADReal & equiv_stress)
{
  using std::sqrt;

  if (_intermediate_porosity == 0.0)
    gauge_stress = equiv_stress;
  else if (_hydro_stress == 0.0)
    gauge_stress = equiv_stress * sqrt(1.0 + 2.0 * _intermediate_porosity / 3.0) /
                   sqrt(1.0 - (1.0 + _power_factor) * _intermediate_porosity +
                        _power_factor * Utility::pow<2>(_intermediate_porosity));
  else
    returnMappingSolve(equiv_stress, gauge_stress, _console);

  mooseAssert(gauge_stress >= equiv_stress,
              "Gauge stress calculated in inner Newton solve is less than the equivalent stress.");
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
  using std::pow;

  computeGaugeStress(gauge_stress, equiv_stress);

  // Compute stress potential
  dpsi_dgauge = _coefficient[_qp] * pow(gauge_stress, _power);

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
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  ADSingleVariableReturnMappingSolution::outputIterationSummary(iter_output, total_it);
}

Real
ADViscoplasticityStressUpdate::computeReferenceResidual(const ADReal & /*effective_trial_stress*/,
                                                        const ADReal & gauge_stress)
{
  // Use gauge stress for relative tolerance criteria, defined as:
  // abs(residual / gauge_stress) <= _relative_tolerance
  return MetaPhysicL::raw_value(gauge_stress);
}
