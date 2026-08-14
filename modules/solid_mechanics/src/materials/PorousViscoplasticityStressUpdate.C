//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousViscoplasticityStressUpdate.h"

#include "libmesh/utility.h"

registerMooseObjectRenamed("SolidMechanicsApp",
                           ADViscoplasticityStressUpdate,
                           "09/30/2027 24:00",
                           ADPorousViscoplasticityStressUpdate);

registerMooseObject("SolidMechanicsApp", PorousViscoplasticityStressUpdate);
registerMooseObject("SolidMechanicsApp", ADPorousViscoplasticityStressUpdate);

template <bool is_ad>
InputParameters
PorousViscoplasticityStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = ViscoplasticityStressUpdateBaseTempl<is_ad>::validParams();
  params += SingleVariableReturnMappingSolutionTempl<is_ad>::validParams();

  params.addClassDescription(
      "Computes the nonlinear homogenized gauge stress and associated viscoplastic response "
      "of a porous material. An optional gas porosity pressure may be included in the hydrostatic "
      "driving stress.");

  MooseEnum viscoplasticity_model("LPS GTN", "LPS");
  params.addParam<MooseEnum>(
      "viscoplasticity_model", viscoplasticity_model, "Which viscoplastic model to use");
  MooseEnum pore_shape_model("spherical cylindrical", "spherical");
  params.addParam<MooseEnum>("pore_shape_model", pore_shape_model, "Which pore shape model to use");
  params.addRequiredParam<MaterialPropertyName>(
      "coefficient", "Material property name for the leading coefficient for Norton power law");
  params.addRequiredRangeCheckedParam<Real>(
      "power", "power>=1.0", "Stress exponent for Norton power law");
  params.addParam<MaterialPropertyName>(
      "additional_porosity_pressure",
      "Optional material property containing additional pressure in the porosity. Positive "
      "pressure adds to the tension-positive matrix hydrostatic stress. The pressure must use the "
      "same stress units as the constitutive model.");
  params.addParam<Real>(
      "maximum_gauge_ratio",
      1.0e6,
      "Maximum ratio between the gauge stress and the equivalent stress/pressure scale. This "
      "should be a high number. It does not directly cap the converged value, but supplies a "
      "range to the inner Newton solve.");

  params.addParam<Real>("minimum_equivalent_stress",
                        1.0e-3,
                        "Minimum stress scale below which viscoplasticity is not calculated.");
  params.renameParam("minimum_equivalent_stress",
                     "minimum_stress_magnitude",
                     "Minimum value of equivalent or absolute value of the hydrostatic stress "
                     "below which viscoplasticity is not calculated.");

  params.addParam<Real>(
      "maximum_equivalent_stress",
      1.0e12,
      "Maximum value of equivalent or absolute value of the hydrostatic stress above which an "
      "exception is thrown instead of calculating the properties in this material.");
  params.renameParam(
      "maximum_equivalent_stress",
      "maximum_stress_magnitude",
      "Maximum value of equivalent or absolute value of the hydrostatic stress above which an "
      "exception is thrown instead of calculating the properties in this material.");

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
      "verbose maximum_gauge_ratio maximum_stress_magnitude use_substepping "
      "substep_strain_tolerance adaptive_substepping maximum_number_substeps",
      "Advanced");

  return params;
}

template <bool is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::PorousViscoplasticityStressUpdateTempl(
    const InputParameters & parameters)
  : ViscoplasticityStressUpdateBaseTempl<is_ad>(parameters),
    SingleVariableReturnMappingSolutionTempl<is_ad>(parameters),
    _model(this->template getParam<MooseEnum>("viscoplasticity_model")
               .template getEnum<ViscoplasticityModel>()),
    _pore_shape(
        this->template getParam<MooseEnum>("pore_shape_model").template getEnum<PoreShapeModel>()),
    _pore_shape_factor(_pore_shape == PoreShapeModel::SPHERICAL ? 1.5 : std::sqrt(3.0)),
    _power(this->template getParam<Real>("power")),
    _power_factor(_model == ViscoplasticityModel::LPS ? (_power - 1.0) / (_power + 1.0) : 1.0),
    _coefficient(this->template getGenericMaterialProperty<Real, is_ad>("coefficient")),
    _additional_porosity_pressure(this->isParamValid("additional_porosity_pressure")
                                      ? &this->template getGenericMaterialProperty<Real, is_ad>(
                                            "additional_porosity_pressure")
                                      : nullptr),
    _gauge_stress(
        this->template declareGenericProperty<Real, is_ad>(this->_base_name + "gauge_stress")),
    _maximum_gauge_ratio(this->template getParam<Real>("maximum_gauge_ratio")),
    _minimum_stress_magnitude(this->template getParam<Real>("minimum_stress_magnitude")),
    _maximum_stress_magnitude(this->template getParam<Real>("maximum_stress_magnitude")),
    _use_substepping(
        this->template getParam<MooseEnum>("use_substepping").template getEnum<SubsteppingType>()),
    _substep_tolerance(this->template getParam<Real>("substep_strain_tolerance")),
    _adaptive_substepping(this->template getParam<bool>("adaptive_substepping")),
    _maximum_number_substeps(this->template getParam<unsigned int>("maximum_number_substeps")),
    _dt_original(0.0),
    _hydro_stress(0.0),
    _identity_two(RankTwoTensor::initIdentity),
    _dhydro_stress_dsigma(_identity_two / 3.0),
    _derivative(0.0)
{
  this->_check_range = true;

  if (parameters.isParamSetByUser("maximum_number_substeps") &&
      _use_substepping == SubsteppingType::NONE)
    this->paramError("maximum_number_substeps",
                     "maximum_number_substeps can only be used when use_substepping is enabled.");

  if (_adaptive_substepping && _use_substepping == SubsteppingType::NONE)
    this->paramError("adaptive_substepping",
                     "adaptive_substepping can only be used when use_substepping is enabled.");

  if (_additional_porosity_pressure && _pore_shape != PoreShapeModel::SPHERICAL)
    this->paramError("additional_porosity_pressure",
                     "The gas-filled-pore implementation is restricted to spherical pores.");

  if (_additional_porosity_pressure && _model != ViscoplasticityModel::LPS)
    this->paramError("additional_porosity_pressure",
                     "The gas-filled-pore implementation is restricted to the LPS model.");
}

template <bool is_ad>
bool
PorousViscoplasticityStressUpdateTempl<is_ad>::substeppingCapabilityEnabled()
{
  return _use_substepping != SubsteppingType::NONE;
}

template <bool is_ad>
bool
PorousViscoplasticityStressUpdateTempl<is_ad>::substeppingCapabilityRequested()
{
  return _use_substepping != SubsteppingType::NONE;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::resetIncrementalMaterialProperties()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp];
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::effectiveHydroStress() const
{
  auto effective_hydro_stress = _hydro_stress;
  if (_additional_porosity_pressure)
    effective_hydro_stress += (*_additional_porosity_pressure)[_qp];

  return effective_hydro_stress;
}

template <bool is_ad>
bool
PorousViscoplasticityStressUpdateTempl<is_ad>::hasViscoplasticDrive(
    const GenericReal<is_ad> & equiv_stress) const
{
  using std::abs;

  if (equiv_stress > _minimum_stress_magnitude)
    return true;

  return _intermediate_porosity > 0.0 && abs(effectiveHydroStress()) > _minimum_stress_magnitude;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::gaugeStressScale(
    const GenericReal<is_ad> & equiv_stress) const
{
  using std::abs;

  auto scale = equiv_stress;
  const auto hydro_scale = abs(effectiveHydroStress());

  if (hydro_scale > scale)
    scale = hydro_scale;

  if (scale < _minimum_stress_magnitude)
    scale = _minimum_stress_magnitude;

  return scale;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::updateState(
    GenericRankTwoTensor<is_ad> & elastic_strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericRankTwoTensor<is_ad> & /*rotation_increment*/,
    GenericRankTwoTensor<is_ad> & stress,
    const RankTwoTensor & /*stress_old*/,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    bool /*compute_full_tangent_operator*/,
    RankFourTensor & /*tangent_operator*/)
{
  updateIntermediatePorosity(elastic_strain_increment);
  resetIncrementalMaterialProperties();
  inelastic_strain_increment.zero();

  const GenericRankTwoTensor<is_ad> elastic_strain_old_ad = elastic_strain_old;
  GenericReal<is_ad> effective_inelastic_strain_increment = 0.0;

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

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::updateStateOneStep(
    GenericRankTwoTensor<is_ad> & elastic_strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    GenericRankTwoTensor<is_ad> & stress,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
    const GenericRankTwoTensor<is_ad> & elastic_strain_old,
    GenericReal<is_ad> & effective_inelastic_strain_increment)
{
  using std::sqrt;

  // Compute the matrix hydrostatic stress. Positive hydrostatic stress is tension.
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
  if (gaugeStressScale(equiv_stress) > _maximum_stress_magnitude)
    mooseException("In ",
                   _name,
                   ": equivalent stress (",
                   MetaPhysicL::raw_value(gaugeStressScale(equiv_stress)),
                   ") is higher than maximum_stress_magnitude (",
                   _maximum_stress_magnitude,
                   ").\nCutting time step.");

  if (hasViscoplasticDrive(equiv_stress))
  {
    GenericReal<is_ad> dpsi_dgauge = 0.0;

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

  // Cut timestep if new stress is higher by a non-numerical noise amount. An additive 1 Pa is
  // utilized ot protect for small differences and if equiv_stresses are zero
  if (new_equiv_stress > equiv_stress + 1.0)
    mooseException("In ",
                   _name,
                   ": updated equivalent stress (",
                   MetaPhysicL::raw_value(new_equiv_stress),
                   ") is greater than initial equivalent stress (",
                   MetaPhysicL::raw_value(equiv_stress),
                   "). Increase the number of local substeps or decrease the global time step.");
}

template <bool is_ad>
unsigned int
PorousViscoplasticityStressUpdateTempl<is_ad>::estimateNumberSubsteps(
    const GenericRankTwoTensor<is_ad> & stress)
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

  if (!hasViscoplasticDrive(equiv_stress))
    return 1;

  if (gaugeStressScale(equiv_stress) > _maximum_stress_magnitude)
    mooseException("In ",
                   _name,
                   ": equivalent stress (",
                   MetaPhysicL::raw_value(gaugeStressScale(equiv_stress)),
                   ") is higher than maximum_stress_magnitude (",
                   _maximum_stress_magnitude,
                   ").\nCutting time step.");

  GenericReal<is_ad> gauge_stress;
  computeGaugeStress(gauge_stress, equiv_stress);

  const auto dpsi_dgauge = _coefficient[_qp] * pow(gauge_stress, _power);
  const auto estimated_effective_increment = std::abs(MetaPhysicL::raw_value(dpsi_dgauge)) * _dt;
  const auto target_increment = _substep_tolerance * this->_max_inelastic_increment;

  if (estimated_effective_increment <= target_increment)
    return 1;

  return static_cast<unsigned int>(ceil(estimated_effective_increment / target_increment));
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::updateStateSubstepInternal(
    GenericRankTwoTensor<is_ad> & strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericRankTwoTensor<is_ad> & rotation_increment,
    GenericRankTwoTensor<is_ad> & stress_new,
    const RankTwoTensor & stress_old,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    unsigned int total_number_substeps,
    bool compute_full_tangent_operator,
    RankFourTensor & tangent_operator)
{
  if (total_number_substeps == 0)
    mooseError("PorousViscoplasticityStressUpdate received zero substeps.");

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

  GenericRankTwoTensor<is_ad> sub_elastic_strain_old = elastic_strain_old;
  auto sub_stress_new = elasticity_tensor * sub_elastic_strain_old;

  strain_increment.zero();
  inelastic_strain_increment.zero();
  stress_new.zero();

  GenericReal<is_ad> accumulated_effective_inelastic_strain_increment = 0.0;

  for (unsigned int step = 0; step < total_number_substeps; ++step)
  {
    auto sub_strain_increment = strain_increment_per_step;
    GenericRankTwoTensor<is_ad> sub_inelastic_strain_increment;
    sub_inelastic_strain_increment.zero();

    sub_stress_new += elasticity_tensor * sub_strain_increment;

    GenericReal<is_ad> sub_effective_inelastic_strain_increment = 0.0;

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
      Moose::out << "PorousViscoplasticityStressUpdateTempl<is_ad> substep " << step + 1 << "/"
                 << total_number_substeps << " dt_sub = " << _dt
                 << " effective inelastic increment = "
                 << MetaPhysicL::raw_value(sub_effective_inelastic_strain_increment)
                 << " effective hydrostatic stress = "
                 << MetaPhysicL::raw_value(effectiveHydroStress()) << std::endl;
  }

  stress_new = sub_stress_new;

  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + accumulated_effective_inelastic_strain_increment;
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp] + inelastic_strain_increment;

  computeStressFinalize(inelastic_strain_increment);
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::updateStateSubstep(
    GenericRankTwoTensor<is_ad> & strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericRankTwoTensor<is_ad> & rotation_increment,
    GenericRankTwoTensor<is_ad> & stress_new,
    const RankTwoTensor & stress_old,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
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

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::initialGuess(
    const GenericReal<is_ad> & effective_trial_stress)
{
  return gaugeStressScale(effective_trial_stress);
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::maximumPermissibleValue(
    const GenericReal<is_ad> & effective_trial_stress) const
{
  return gaugeStressScale(effective_trial_stress) * _maximum_gauge_ratio;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::minimumPermissibleValue(
    const GenericReal<is_ad> & effective_trial_stress) const
{
  /*
   * Lambda must remain positive because M contains 1/Lambda. Retain Lambda >= q,
   * but when q=0 use a small positive floor based on the pressure/deviatoric
   * stress scale. Increasing maximum_gauge_ratio widens this admissible range.
   */
  GenericReal<is_ad> minimum = effective_trial_stress;
  const GenericReal<is_ad> positive_floor =
      gaugeStressScale(effective_trial_stress) / _maximum_gauge_ratio;

  if (positive_floor > minimum)
    minimum = positive_floor;

  return minimum;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeResidual(
    const GenericReal<is_ad> & equiv_stress, const GenericReal<is_ad> & trial_gauge)
{
  using std::abs;
  using std::cosh;
  using std::sinh;

  const auto effective_hydro_stress = effectiveHydroStress();
  const auto M = abs(effective_hydro_stress) / trial_gauge;
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
    Moose::out << "in computeResidual:\n"
               << "  position: " << _q_point[_qp]
               << " matrix_hydro_stress: " << MetaPhysicL::raw_value(_hydro_stress)
               << " effective_hydro_stress: " << MetaPhysicL::raw_value(effective_hydro_stress)
               << " equiv_stress: " << MetaPhysicL::raw_value(equiv_stress)
               << " trial_gauge: " << MetaPhysicL::raw_value(trial_gauge)
               << " M: " << MetaPhysicL::raw_value(M)
               << "\n  residual: " << MetaPhysicL::raw_value(residual)
               << " derivative: " << MetaPhysicL::raw_value(_derivative) << std::endl;

  return residual;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeH(const Real n,
                                                        const GenericReal<is_ad> & M,
                                                        const bool derivative)
{
  using std::pow;

  const auto mod = pow(M * _pore_shape_factor, (n + 1.0) / n);

  if (derivative)
  {
    if (M == 0.0)
      return 0.0;

    const auto dmod_dM = (n + 1.0) / n * mod / M;
    return dmod_dM * pow(1.0 + mod / n, n - 1.0);
  }

  return pow(1.0 + mod / n, n);
}

template <bool is_ad>
GenericRankTwoTensor<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeDGaugeDSigma(
    const GenericReal<is_ad> & gauge_stress,
    const GenericReal<is_ad> & equiv_stress,
    const GenericRankTwoTensor<is_ad> & dev_stress,
    const GenericRankTwoTensor<is_ad> & /*stress*/)
{
  using std::abs;
  using std::sinh;

  const auto effective_hydro_stress = effectiveHydroStress();
  const auto M = abs(effective_hydro_stress) / gauge_stress;
  const auto h = computeH(_power, M);

  /* Partial derivative of F with respect to the effective hydrostatic stress.
   *
   * Porosity pressure is treated as an internal/material quantity held fixed in
   * this partial derivative, therefore:
   *
   *   d(sigma_h + p_b)/d(sigma) = I/3
   *
   * for spherical pores.
   */
  GenericReal<is_ad> dresidual_deffective_hydro_stress = 0.0;

  if (effective_hydro_stress != 0.0)
  {
    const auto dM_deffective_hydro_stress = M / effective_hydro_stress;

    if (_model == ViscoplasticityModel::GTN)
    {
      dresidual_deffective_hydro_stress = 2.0 * _intermediate_porosity *
                                          sinh(_pore_shape_factor * M) * _pore_shape_factor *
                                          dM_deffective_hydro_stress;
    }
    else
    {
      const auto dresidual_dh = _intermediate_porosity * (1.0 - _power_factor / Utility::pow<2>(h));
      const auto dh_dM = computeH(_power, M, true);

      dresidual_deffective_hydro_stress = dresidual_dh * dh_dM * dM_deffective_hydro_stress;
    }
  }

  // Combine dF/dq and dq/dsigma analytically:
  // dF/dq * dq/dsigma = (2 A q / Lambda^2) * (3/2 s/q) = 3 A s / Lambda^2
  auto dresidual_dequiv_stress_dequiv_stress_dsigma =
      3.0 * dev_stress / Utility::pow<2>(gauge_stress);

  if (_pore_shape == PoreShapeModel::SPHERICAL)
    dresidual_dequiv_stress_dequiv_stress_dsigma *= 1.0 + _intermediate_porosity / 1.5;

  const GenericRankTwoTensor<is_ad> dresidual_dsigma =
      dresidual_deffective_hydro_stress * _dhydro_stress_dsigma +
      dresidual_dequiv_stress_dequiv_stress_dsigma;

  /* Re-evaluate the residual at the converged gauge stress to obtain dF/dLambda.
   * This is required for branches in computeGaugeStress() that obtain Lambda
   * analytically instead of through returnMappingSolve().
   */
  computeResidual(equiv_stress, gauge_stress);
  const auto dresidual_dgauge = _derivative;

  return dresidual_dsigma * (-1.0 / dresidual_dgauge);
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::computeGaugeStress(
    GenericReal<is_ad> & gauge_stress, const GenericReal<is_ad> & equiv_stress)
{
  using std::sqrt;

  const auto effective_hydro_stress = effectiveHydroStress();

  if (_intermediate_porosity == 0.0)
    gauge_stress = equiv_stress;
  else if (effective_hydro_stress == 0.0)
    gauge_stress = equiv_stress * sqrt(1.0 + 2.0 * _intermediate_porosity / 3.0) /
                   sqrt(1.0 - (1.0 + _power_factor) * _intermediate_porosity +
                        _power_factor * Utility::pow<2>(_intermediate_porosity));
  else
    this->returnMappingSolve(equiv_stress, gauge_stress, _console);

  mooseAssert(gauge_stress >= equiv_stress,
              "Gauge stress calculated in inner Newton solve is less than the equivalent stress.");
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::computeInelasticStrainIncrement(
    GenericReal<is_ad> & gauge_stress,
    GenericReal<is_ad> & dpsi_dgauge,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericReal<is_ad> & equiv_stress,
    const GenericRankTwoTensor<is_ad> & dev_stress,
    const GenericRankTwoTensor<is_ad> & stress)
{
  using std::pow;

  computeGaugeStress(gauge_stress, equiv_stress);

  dpsi_dgauge = _coefficient[_qp] * pow(gauge_stress, _power);

  inelastic_strain_increment =
      _dt * dpsi_dgauge * computeDGaugeDSigma(gauge_stress, equiv_stress, dev_stress, stress);
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::outputIterationSummary(
    std::stringstream * iter_output, const unsigned int total_it)
{
  if (iter_output)
    *iter_output << "At element " << this->_current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << this->_current_elem->subdomain_id() << '\n';

  SingleVariableReturnMappingSolutionTempl<is_ad>::outputIterationSummary(iter_output, total_it);
}

template <bool is_ad>
Real
PorousViscoplasticityStressUpdateTempl<is_ad>::computeReferenceResidual(
    const GenericReal<is_ad> & /*effective_trial_stress*/, const GenericReal<is_ad> & gauge_stress)
{
  // Use gauge stress for relative tolerance criteria, defined as:
  // abs(residual / gauge_stress) <= _relative_tolerance
  return MetaPhysicL::raw_value(gauge_stress);
}

template class PorousViscoplasticityStressUpdateTempl<false>;
template class PorousViscoplasticityStressUpdateTempl<true>;
