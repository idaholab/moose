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
#include "MooseUtils.h"

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
      "of a porous material. One or more Norton creep laws may be supplied; each exponent is "
      "evaluated on its own exponent-dependent porous gauge surface. An optional gas porosity "
      "pressure may be included in the hydrostatic driving stress.");
  MooseEnum viscoplasticity_model("LPS GTN", "LPS");
  params.addParam<MooseEnum>(
      "viscoplasticity_model", viscoplasticity_model, "Which viscoplastic model to use");
  MooseEnum pore_shape_model("spherical cylindrical", "spherical");
  params.addParam<MooseEnum>("pore_shape_model", pore_shape_model, "Which pore shape model to use");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "coefficient",
      "One or more material property names for the nonnegative leading coefficients of Norton "
      "power laws. An exactly zero coefficient disables that mechanism without solving its gauge "
      "surface.");
  params.addRequiredParam<std::vector<Real>>(
      "power", "One or more stress exponents for the corresponding Norton power laws");
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
      "Use local constitutive substeps controlled by the viscoplastic strain increment.");
  params.addParam<MooseEnum>(
      "use_substepping", substepping_type, "Whether and how to use local constitutive substepping");
  params.addRangeCheckedParam<Real>(
      "substep_strain_tolerance",
      0.1,
      "substep_strain_tolerance>0.0",
      "Target fraction of max_inelastic_increment allowed in one local substep. A value of 1 uses "
      "max_inelastic_increment itself as the local target. Fixed increment-based substepping sizes "
      "the step from the elastic-trial estimate; adaptive substepping initializes from the "
      "previous "
      "accepted effective inelastic rate and enforces this target using converged local "
      "increments.");
  params.addParam<bool>(
      "adaptive_substepping",
      false,
      "Initialize the substep count from the previous accepted global-step effective inelastic "
      "rate, using one substep when that history is zero, and adaptively refine when the local "
      "solve "
      "fails or a converged local inelastic increment exceeds substep_strain_tolerance times "
      "max_inelastic_increment. The elastic-trial predictor is not used in adaptive mode.");
  params.addRangeCheckedParam<unsigned int>(
      "maximum_number_substeps",
      25,
      "maximum_number_substeps>=1",
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
    _effective_inelastic_strain_rate(this->template declareGenericProperty<Real, is_ad>(
        this->_base_name + "effective_" +
        this->template getParam<std::string>("inelastic_strain_name") + "_rate")),
    _effective_inelastic_strain_rate_old(this->template getMaterialPropertyOld<Real>(
        this->_base_name + "effective_" +
        this->template getParam<std::string>("inelastic_strain_name") + "_rate")),
    _suggested_number_substeps(0),
    _last_effective_inelastic_strain_increment(0.0),
    _hydro_stress(0.0),
    _identity_two(RankTwoTensor::initIdentity),
    _dhydro_stress_dsigma(_identity_two / 3.0),
    _derivative(0.0)
{
  this->_check_range = true;

  const auto & coefficient_names =
      this->template getParam<std::vector<MaterialPropertyName>>("coefficient");
  const auto & powers = this->template getParam<std::vector<Real>>("power");

  if (coefficient_names.empty())
    this->paramError("coefficient", "At least one creep coefficient must be supplied.");
  if (powers.empty())
    this->paramError("power", "At least one creep exponent must be supplied.");
  if (coefficient_names.size() != powers.size())
    this->paramError("power",
                     "The number of power entries must equal the number of coefficient entries.");

  _creep_laws.reserve(powers.size());
  for (auto i = std::size_t{0}; i < powers.size(); ++i)
  {
    if (!std::isfinite(powers[i]) || powers[i] < 1.0)
      this->paramError("power",
                       "Every Norton power must be finite and greater than or equal to 1.0.");

    const auto power_factor =
        _model == ViscoplasticityModel::LPS ? (powers[i] - 1.0) / (powers[i] + 1.0) : 1.0;
    _creep_laws.push_back(
        {&this->template getPossiblyConstantGenericMaterialPropertyByName<Real, is_ad>(
             coefficient_names[i], this->_material_data, 0),
         powers[i],
         power_factor});
  }

  if (_creep_laws.size() > 1)
  {
    _gauge_stress_laws.reserve(_creep_laws.size());
    for (auto i = std::size_t{0}; i < _creep_laws.size(); ++i)
      _gauge_stress_laws.push_back(&this->template declareGenericProperty<Real, is_ad>(
          this->_base_name + "gauge_stress_" + std::to_string(i)));
  }

  if (parameters.isParamSetByUser("maximum_number_substeps") &&
      _use_substepping == SubsteppingType::NONE)
    this->paramError("maximum_number_substeps",
                     "maximum_number_substeps can only be used when use_substepping is enabled.");

  if (_adaptive_substepping && _use_substepping == SubsteppingType::NONE)
    this->paramError("adaptive_substepping",
                     "adaptive_substepping can only be used when use_substepping is enabled.");

  if (_use_substepping != SubsteppingType::NONE && this->_max_inelastic_increment <= 0.0)
    this->paramError("max_inelastic_increment",
                     "max_inelastic_increment must be positive when substepping is enabled.");

  if (_additional_porosity_pressure && _pore_shape != PoreShapeModel::SPHERICAL)
    this->paramError("additional_porosity_pressure",
                     "The gas-filled-pore implementation is restricted to spherical pores.");

  if (_additional_porosity_pressure && _model != ViscoplasticityModel::LPS)
    this->paramError("additional_porosity_pressure",
                     "The gas-filled-pore implementation is restricted to the LPS model.");
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::creepCoefficient(const std::size_t law_index) const
{
  mooseAssert(law_index < _creep_laws.size(), "Creep-law index is out of range.");

  const auto coefficient = (*_creep_laws[law_index].coefficient)[_qp];
  const auto coefficient_raw = MetaPhysicL::raw_value(coefficient);
  if (!std::isfinite(coefficient_raw) || coefficient_raw < 0.0)
    mooseException("In ",
                   _name,
                   ": Norton coefficient for creep law ",
                   law_index,
                   " must be finite and nonnegative. coefficient = ",
                   coefficient_raw,
                   ".");

  return coefficient;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeCreepRate(
    const CreepLaw & law,
    const GenericReal<is_ad> & coefficient,
    const GenericReal<is_ad> & gauge_stress) const
{
  using std::pow;
  return coefficient * pow(gauge_stress, law.power);
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::setGaugeStress(
    const std::size_t law_index, const GenericReal<is_ad> & gauge_stress)
{
  mooseAssert(law_index < _creep_laws.size(), "Creep-law index is out of range.");

  if (!_gauge_stress_laws.empty())
    (*_gauge_stress_laws[law_index])[_qp] = gauge_stress;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::setGaugeStresses(
    const GenericReal<is_ad> & equiv_stress,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & porosity)
{
  const auto has_drive = hasViscoplasticDrive(equiv_stress, effective_hydro_stress, porosity);
  auto primary_set = false;
  _gauge_stress[_qp] = 0.0;

  for (auto law_index = std::size_t{0}; law_index < _creep_laws.size(); ++law_index)
  {
    const auto coefficient = creepCoefficient(law_index);
    const auto zero_coefficient = MetaPhysicL::raw_value(coefficient) == 0.0;
    // Preserve the legacy single-law gauge diagnostic when C = 0. In a multi-law model, an
    // exactly zero coefficient denotes an inactive mechanism and its per-law gauge remains zero.
    if (zero_coefficient && _creep_laws.size() > 1)
    {
      setGaugeStress(law_index, GenericReal<is_ad>(0.0));
      continue;
    }

    auto gauge_stress = equiv_stress;
    if (has_drive)
      computeGaugeStress(
          gauge_stress, equiv_stress, effective_hydro_stress, porosity, _creep_laws[law_index]);
    setGaugeStress(law_index, gauge_stress);
    if (!primary_set)
    {
      _gauge_stress[_qp] = gauge_stress;
      primary_set = true;
    }
  }
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
PorousViscoplasticityStressUpdateTempl<is_ad>::initQpStatefulProperties()
{
  ViscoplasticityStressUpdateBaseTempl<is_ad>::initQpStatefulProperties();
  _effective_inelastic_strain_rate[_qp] = 0.0;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::propagateQpStatefulProperties()
{
  ViscoplasticityStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties();
  _effective_inelastic_strain_rate[_qp] = _effective_inelastic_strain_rate_old[_qp];
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::resetIncrementalMaterialProperties()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp];
  _effective_inelastic_strain_rate[_qp] = _effective_inelastic_strain_rate_old[_qp];
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::matrixHydroStress(
    const GenericRankTwoTensor<is_ad> & stress) const
{
  if (_pore_shape == PoreShapeModel::CYLINDRICAL)
    return (stress(0, 0) + stress(1, 1)) / 2.0;
  return stress.trace() / 3.0;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::effectiveHydroStress(
    const GenericReal<is_ad> & matrix_hydro_stress) const
{
  auto effective_hydro_stress = matrix_hydro_stress;
  if (_additional_porosity_pressure)
    effective_hydro_stress += (*_additional_porosity_pressure)[_qp];

  return effective_hydro_stress;
}

template <bool is_ad>
bool
PorousViscoplasticityStressUpdateTempl<is_ad>::hasViscoplasticDrive(
    const GenericReal<is_ad> & equiv_stress,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & porosity) const
{
  using std::abs;

  if (equiv_stress > _minimum_stress_magnitude)
    return true;

  return porosity > 0.0 && abs(effective_hydro_stress) > _minimum_stress_magnitude;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::gaugeStressScale(
    const GenericReal<is_ad> & equiv_stress,
    const GenericReal<is_ad> & effective_hydro_stress) const
{
  using std::abs;

  auto scale = equiv_stress;
  const auto hydro_scale = abs(effective_hydro_stress);

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
  // _dt aliases FEProblem::dt(), so never use it as constitutive scratch storage. Reconstruct the
  // current global step from time - time_old and keep any local subdivision in this object only.
  this->resetConstitutiveTimeStep();

  // Treat one constitutive evaluation as a transaction. Derived stress updates may advance
  // incremental material state during initialization; if any later local solve or finalization
  // throws, restore both caller-owned tensors and virtual incremental state before propagating it.
  const auto original_elastic_strain_increment = elastic_strain_increment;
  const auto original_inelastic_strain_increment = inelastic_strain_increment;
  const auto original_stress = stress;
  const auto original_intermediate_porosity = _intermediate_porosity;
  const auto original_hydro_stress = _hydro_stress;
  const auto original_gauge_stress = _gauge_stress[_qp];
  std::vector<GenericReal<is_ad>> original_gauge_stresses;
  original_gauge_stresses.reserve(_gauge_stress_laws.size());
  for (const auto * gauge_stress : _gauge_stress_laws)
    original_gauge_stresses.push_back((*gauge_stress)[_qp]);

  const auto restore_attempt_state = [&]()
  {
    elastic_strain_increment = original_elastic_strain_increment;
    inelastic_strain_increment = original_inelastic_strain_increment;
    stress = original_stress;
    resetIncrementalMaterialProperties();
    _intermediate_porosity = original_intermediate_porosity;
    _hydro_stress = original_hydro_stress;
    _gauge_stress[_qp] = original_gauge_stress;
    for (auto law_index = std::size_t{0}; law_index < _gauge_stress_laws.size(); ++law_index)
      (*_gauge_stress_laws[law_index])[_qp] = original_gauge_stresses[law_index];
  };

  try
  {
    this->updateIntermediatePorosity(elastic_strain_increment);
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
    _last_effective_inelastic_strain_increment =
        std::abs(MetaPhysicL::raw_value(effective_inelastic_strain_increment));
    _effective_inelastic_strain[_qp] =
        _effective_inelastic_strain_old[_qp] + effective_inelastic_strain_increment;
    _inelastic_strain[_qp] = _inelastic_strain_old[_qp] + inelastic_strain_increment;

    this->computeStressFinalize(inelastic_strain_increment);
    recordEffectiveInelasticStrainRate(effective_inelastic_strain_increment);
  }
  catch (...)
  {
    restore_attempt_state();
    throw;
  }
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
  _hydro_stress = matrixHydroStress(stress);
  // Compute intermediate equivalent stress
  const auto dev_stress = stress.deviatoric();
  const auto dev_stress_squared = dev_stress.doubleContraction(dev_stress);
  const auto equiv_stress =
      dev_stress_squared == 0.0 ? GenericReal<is_ad>(0.0) : sqrt(1.5 * dev_stress_squared);

  this->computeStressInitialize(equiv_stress, elasticity_tensor);

  // Prepare values
  inelastic_strain_increment.zero();
  effective_inelastic_strain_increment = 0.0;
  const auto effective_hydro_stress = effectiveHydroStress(_hydro_stress);
  const auto stress_scale = gaugeStressScale(equiv_stress, effective_hydro_stress);
  // Protect against extremely high values of stresses calculated by other viscoplastic materials
  if (stress_scale > _maximum_stress_magnitude)
    mooseException("In ",
                   _name,
                   ": equivalent stress (",
                   MetaPhysicL::raw_value(stress_scale),
                   ") is higher than maximum_stress_magnitude (",
                   _maximum_stress_magnitude,
                   ").\nCutting time step.");
  if (hasViscoplasticDrive(equiv_stress, effective_hydro_stress, _intermediate_porosity))
  {
    GenericReal<is_ad> total_creep_rate = 0.0;
    computeInelasticStrainIncrement(total_creep_rate,
                                    inelastic_strain_increment,
                                    equiv_stress,
                                    dev_stress,
                                    effective_hydro_stress,
                                    _intermediate_porosity);
    // Update elastic strain increment due to inelastic strain calculated here
    elastic_strain_increment -= inelastic_strain_increment;
    // Update stress due to new strain
    stress = elasticity_tensor * (elastic_strain_old + elastic_strain_increment);
    _hydro_stress = matrixHydroStress(stress);

    effective_inelastic_strain_increment = total_creep_rate * this->constitutiveTimeStep();
  }
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::advanceSubstepPorosity(
    const GenericRankTwoTensor<is_ad> & inelastic_strain_increment)
{
  // Local constitutive substeps represent smaller internal time increments. Advance this model's
  // porosity between successful substeps so the next creep solve sees the state that would have
  // resulted from taking a smaller global timestep. Retain the accepted global-old (1-f) factor so
  // the accumulated local increments reproduce the existing full-step PorosityFromStrain update.
  _intermediate_porosity +=
      (1.0 - _porosity_old[_qp]) * inelastic_strain_increment.trace();
  this->enforceIntermediatePorosityBounds();
}

template <bool is_ad>
unsigned int
PorousViscoplasticityStressUpdateTempl<is_ad>::estimateNumberSubsteps(
    const GenericRankTwoTensor<is_ad> & stress)
{
  const auto matrix_hydro_stress = matrixHydroStress(stress);

  return estimateNumberSubstepsFromState(
      stress, effectiveHydroStress(matrix_hydro_stress), _intermediate_porosity);
}

template <bool is_ad>
unsigned int
PorousViscoplasticityStressUpdateTempl<is_ad>::estimateNumberSubstepsFromState(
    const GenericRankTwoTensor<is_ad> & stress,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & porosity)
{
  using std::sqrt;

  const auto dev_stress = stress.deviatoric();
  const auto dev_stress_squared = dev_stress.doubleContraction(dev_stress);
  const auto equiv_stress =
      dev_stress_squared == 0.0 ? GenericReal<is_ad>(0.0) : sqrt(1.5 * dev_stress_squared);
  if (!hasViscoplasticDrive(equiv_stress, effective_hydro_stress, porosity))
    return 1;

  const auto stress_scale = gaugeStressScale(equiv_stress, effective_hydro_stress);
  if (stress_scale > _maximum_stress_magnitude)
    mooseException("In ",
                   _name,
                   ": equivalent stress (",
                   MetaPhysicL::raw_value(stress_scale),
                   ") is higher than maximum_stress_magnitude (",
                   _maximum_stress_magnitude,
                   ") while estimating local substeps.\nCutting time step.");

  auto estimated_effective_increment = Real(0.0);
  for (auto law_index = std::size_t{0}; law_index < _creep_laws.size(); ++law_index)
  {
    const auto coefficient = creepCoefficient(law_index);
    if (MetaPhysicL::raw_value(coefficient) == 0.0)
      continue;

    const auto & law = _creep_laws[law_index];
    GenericReal<is_ad> gauge_stress;
    computeGaugeStress(gauge_stress, equiv_stress, effective_hydro_stress, porosity, law);
    estimated_effective_increment +=
        std::abs(MetaPhysicL::raw_value(computeCreepRate(law, coefficient, gauge_stress))) *
        this->globalTimeStep();
  }

  if (!std::isfinite(estimated_effective_increment))
    mooseException("In ",
                   _name,
                   ": nonfinite predicted effective inelastic increment while "
                   "estimating local substeps.");

  const auto target_increment = _substep_tolerance * this->_max_inelastic_increment;
  const auto estimated_ratio = estimated_effective_increment / target_increment;
  const auto estimated_number_substeps =
      estimated_ratio <= 1.0 ? 1u
      : estimated_ratio >= std::numeric_limits<unsigned int>::max()
          ? std::numeric_limits<unsigned int>::max()
          : static_cast<unsigned int>(std::ceil(estimated_ratio));

  if (_verbose && estimated_number_substeps > 1)
  {
    Moose::out << "Porous viscoplastic substep predictor at element " << this->_current_elem->id()
               << " _qp=" << _qp << " position=" << _q_point[_qp]
               << " global_dt=" << this->globalTimeStep()
               << " p_eff=" << MetaPhysicL::raw_value(effective_hydro_stress)
               << " q=" << MetaPhysicL::raw_value(equiv_stress)
               << " porosity=" << MetaPhysicL::raw_value(porosity)
               << " target_increment=" << target_increment
               << " estimated_increment=" << estimated_effective_increment
               << " estimated_substeps=" << estimated_number_substeps << '\n';
    for (auto law_index = std::size_t{0}; law_index < _creep_laws.size(); ++law_index)
    {
      const auto coefficient = creepCoefficient(law_index);
      const auto coefficient_raw = MetaPhysicL::raw_value(coefficient);
      if (coefficient_raw == 0.0)
      {
        Moose::out << "  law " << law_index << ": n=" << _creep_laws[law_index].power
                   << " coefficient=0 gauge_stress=0 creep_rate=0 predicted_increment=0\n";
        continue;
      }

      const auto & law = _creep_laws[law_index];
      GenericReal<is_ad> gauge_stress;
      computeGaugeStress(gauge_stress, equiv_stress, effective_hydro_stress, porosity, law);
      const auto creep_rate =
          std::abs(MetaPhysicL::raw_value(computeCreepRate(law, coefficient, gauge_stress)));
      Moose::out << "  law " << law_index << ": n=" << law.power
                 << " coefficient=" << coefficient_raw
                 << " gauge_stress=" << MetaPhysicL::raw_value(gauge_stress)
                 << " creep_rate=" << creep_rate
                 << " predicted_increment=" << creep_rate * this->globalTimeStep() << '\n';
    }
  }

  return estimated_number_substeps;
}

template <bool is_ad>
unsigned int
PorousViscoplasticityStressUpdateTempl<is_ad>::estimateAdaptiveNumberSubstepsFromHistory() const
{
  const auto previous_rate = std::abs(_effective_inelastic_strain_rate_old[_qp]);
  if (!std::isfinite(previous_rate))
    mooseException(
        "In ", _name, ": previous accepted effective inelastic strain rate is nonfinite.");

  const auto target_increment = _substep_tolerance * this->_max_inelastic_increment;
  const auto predicted_increment = previous_rate * this->globalTimeStep();
  if (!std::isfinite(predicted_increment))
    mooseException(
        "In ", _name, ": history-based predicted effective inelastic increment is nonfinite.");

  const auto predicted_ratio = predicted_increment / target_increment;
  auto predicted_substeps = predicted_ratio <= 1.0 ? 1u
                            : predicted_ratio >= std::numeric_limits<unsigned int>::max()
                                ? std::numeric_limits<unsigned int>::max()
                                : static_cast<unsigned int>(std::ceil(predicted_ratio));

  // History is only an initial guess. Never cut the global timestep solely because a lagged
  // predictor exceeds the configured local-substep budget; try the maximum allowed subdivision and
  // let the current converged constitutive response decide whether the global step is admissible.
  predicted_substeps = std::min(predicted_substeps, _maximum_number_substeps);

  if (_verbose && previous_rate > 0.0)
    Moose::out << "In " << _name << ": adaptive history substep predictor at element "
               << this->_current_elem->id() << " _qp=" << _qp << " position=" << _q_point[_qp]
               << " previous_accepted_rate=" << previous_rate
               << " global_dt=" << this->globalTimeStep()
               << " target_increment=" << target_increment
               << " predicted_increment=" << predicted_increment
               << " initial_substeps=" << predicted_substeps << std::endl;

  return predicted_substeps;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::recordEffectiveInelasticStrainRate(
    const GenericReal<is_ad> & effective_inelastic_strain_increment)
{
  if (!_adaptive_substepping)
    return;

  const auto increment = std::abs(MetaPhysicL::raw_value(effective_inelastic_strain_increment));
  if (!std::isfinite(increment))
    mooseException("In ", _name, ": converged effective inelastic strain increment is nonfinite.");

  const auto global_time_step = this->globalTimeStep();
  _effective_inelastic_strain_rate[_qp] =
      global_time_step > 0.0 ? increment / global_time_step : 0.0;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::checkSubstepIncrement(
    const GenericReal<is_ad> & effective_inelastic_strain_increment,
    const unsigned int total_number_substeps,
    const unsigned int substep_index)
{
  if (!_adaptive_substepping)
    return;

  const auto increment = std::abs(MetaPhysicL::raw_value(effective_inelastic_strain_increment));
  if (!std::isfinite(increment))
    mooseException("In ", _name, ": nonfinite effective inelastic increment in local substep.");

  const auto target_increment = _substep_tolerance * this->_max_inelastic_increment;
  if (increment <= target_increment * (1.0 + 1.0e-12))
    return;

  const auto suggested =
      std::ceil(static_cast<Real>(total_number_substeps) * increment / target_increment);
  const auto bounded_suggestion = suggested >= _maximum_number_substeps
                                      ? _maximum_number_substeps
                                      : static_cast<unsigned int>(suggested);
  const auto one_more_substep = total_number_substeps >= _maximum_number_substeps
                                    ? _maximum_number_substeps
                                    : total_number_substeps + 1;
  _suggested_number_substeps = std::max(one_more_substep, bounded_suggestion);

  if (_verbose)
    Moose::out << "In " << _name << ": actual effective inelastic increment " << increment
               << " in local substep " << substep_index << "/" << total_number_substeps
               << " exceeds target " << target_increment << "; suggesting "
               << _suggested_number_substeps << " substeps." << std::endl;

  mooseException("In ",
                 _name,
                 ": actual effective inelastic increment in local substep ",
                 substep_index,
                 "/",
                 total_number_substeps,
                 " is ",
                 increment,
                 ", which exceeds the substep target ",
                 target_increment,
                 ". Suggested retry uses ",
                 _suggested_number_substeps,
                 " substeps.");
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
    _last_effective_inelastic_strain_increment = 0.0;
    updateState(strain_increment,
                inelastic_strain_increment,
                rotation_increment,
                stress_new,
                stress_old,
                elasticity_tensor,
                elastic_strain_old,
                compute_full_tangent_operator,
                tangent_operator);
    checkSubstepIncrement(_last_effective_inelastic_strain_increment, 1, 1);
    return;
  }
  if (total_number_substeps > _maximum_number_substeps)
    mooseException("The number of substeps computed exceeds 'maximum_number_substeps'.");

  this->setConstitutiveTimeStep(this->globalTimeStep() / total_number_substeps);

  const auto strain_increment_per_step = strain_increment / total_number_substeps;

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
    _last_effective_inelastic_strain_increment =
        std::abs(MetaPhysicL::raw_value(sub_effective_inelastic_strain_increment));
    checkSubstepIncrement(
        sub_effective_inelastic_strain_increment, total_number_substeps, step + 1);
    advanceSubstepPorosity(sub_inelastic_strain_increment);

    strain_increment += sub_strain_increment;
    inelastic_strain_increment += sub_inelastic_strain_increment;
    sub_elastic_strain_old += sub_strain_increment;
    sub_stress_new = elasticity_tensor * sub_elastic_strain_old;
    accumulated_effective_inelastic_strain_increment += sub_effective_inelastic_strain_increment;
    if (_verbose)
      Moose::out << "PorousViscoplasticityStressUpdateTempl<is_ad> substep " << step + 1 << "/"
                 << total_number_substeps << " dt_sub = " << this->constitutiveTimeStep()
                 << " global_dt = " << this->globalTimeStep() << " shared_dt = " << _dt
                 << " effective inelastic increment = "
                 << MetaPhysicL::raw_value(sub_effective_inelastic_strain_increment)
                 << " effective hydrostatic stress = "
                 << MetaPhysicL::raw_value(effectiveHydroStress(_hydro_stress)) << std::endl;
  }

  stress_new = sub_stress_new;
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + accumulated_effective_inelastic_strain_increment;
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp] + inelastic_strain_increment;

  this->computeStressFinalize(inelastic_strain_increment);
  recordEffectiveInelasticStrainRate(accumulated_effective_inelastic_strain_increment);
  this->resetConstitutiveTimeStep();
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
  const auto global_time_step = this->globalTimeStep();
  this->setConstitutiveTimeStep(global_time_step);

  // Adaptive retries must begin from exactly the same accepted material state. This is especially
  // important for derived models that integrate internal kinetics over each constitutive substep.
  const auto original_strain_increment = strain_increment;
  const auto original_inelastic_strain_increment = inelastic_strain_increment;
  const auto original_stress_new = stress_new;
  const auto original_intermediate_porosity = _intermediate_porosity;
  const auto original_hydro_stress = _hydro_stress;
  const auto original_gauge_stress = _gauge_stress[_qp];
  std::vector<GenericReal<is_ad>> original_gauge_stresses;
  original_gauge_stresses.reserve(_gauge_stress_laws.size());
  for (const auto * gauge_stress : _gauge_stress_laws)
    original_gauge_stresses.push_back((*gauge_stress)[_qp]);

  const auto restore_attempt_state = [&]()
  {
    this->setConstitutiveTimeStep(global_time_step);
    strain_increment = original_strain_increment;
    inelastic_strain_increment = original_inelastic_strain_increment;
    stress_new = original_stress_new;
    resetIncrementalMaterialProperties();
    _intermediate_porosity = original_intermediate_porosity;
    _hydro_stress = original_hydro_stress;
    _gauge_stress[_qp] = original_gauge_stress;
    for (auto law_index = std::size_t{0}; law_index < _gauge_stress_laws.size(); ++law_index)
      (*_gauge_stress_laws[law_index])[_qp] = original_gauge_stresses[law_index];
  };

  // Initialize this model's substep porosity from inelastic increments already computed by other
  // inelastic models. Successful local substeps then advance it through advanceSubstepPorosity().
  this->updateIntermediatePorosity(original_strain_increment);

  unsigned int number_substeps;
  if (_adaptive_substepping)
    number_substeps = estimateAdaptiveNumberSubstepsFromHistory();
  else
    // Preserve the historical fixed INCREMENT_BASED behavior when adaptive retries are disabled.
    number_substeps = estimateNumberSubsteps(original_stress_new);

  while (true)
  {
    if (number_substeps > _maximum_number_substeps)
    {
      restore_attempt_state();
      mooseException("In ",
                     _name,
                     ": estimated number of viscoplastic substeps (",
                     number_substeps,
                     ") exceeds maximum_number_substeps (",
                     _maximum_number_substeps,
                     "). Cutting global time step.");
    }

    strain_increment = original_strain_increment;
    inelastic_strain_increment.zero();
    stress_new = original_stress_new;
    _gauge_stress[_qp] = original_gauge_stress;
    for (auto law_index = std::size_t{0}; law_index < _gauge_stress_laws.size(); ++law_index)
      (*_gauge_stress_laws[law_index])[_qp] = original_gauge_stresses[law_index];
    resetIncrementalMaterialProperties();
    this->updateIntermediatePorosity(original_strain_increment);
    _suggested_number_substeps = 0;

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
      this->setConstitutiveTimeStep(global_time_step);
      return;
    }
    catch (...)
    {
      const auto suggested_number_substeps = _suggested_number_substeps;
      restore_attempt_state();

      if (!_adaptive_substepping)
        throw;

      if (number_substeps >= _maximum_number_substeps)
        break;

      if (suggested_number_substeps > number_substeps)
        number_substeps = suggested_number_substeps;
      else
        number_substeps = number_substeps > _maximum_number_substeps / 2 ? _maximum_number_substeps
                                                                         : 2 * number_substeps;

      if (_verbose)
        Moose::out << "In " << _name << ": retrying adaptive viscoplastic integration with "
                   << number_substeps << " substeps." << std::endl;
    }
  }

  restore_attempt_state();
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
  using std::abs;
  using std::pow;
  using std::sqrt;

  const auto scale =
      gaugeStressScale(effective_trial_stress, _gauge_solve_state.effective_hydro_stress);
  if (_model != ViscoplasticityModel::LPS || !_gauge_solve_state.law ||
      _gauge_solve_state.porosity <= 0.0)
    return scale;

  const auto & law = *_gauge_solve_state.law;
  const auto & f = _gauge_solve_state.porosity;
  const auto alpha = law.power_factor;
  const auto A =
      _pore_shape == PoreShapeModel::SPHERICAL ? 1.0 + 2.0 * f / 3.0 : GenericReal<is_ad>(1.0);
  const auto denominator = 1.0 - (1.0 + alpha) * f + alpha * Utility::pow<2>(f);

  auto lambda_q = GenericReal<is_ad>(0.0);
  if (denominator > 0.0)
    lambda_q = effective_trial_stress * sqrt(A / denominator);

  auto lambda_h = GenericReal<is_ad>(0.0);
  const auto hydro = abs(_gauge_solve_state.effective_hydro_stress);
  if (hydro > 0.0)
  {
    const auto n_to_n = pow(law.power, law.power);
    lambda_h = _pore_shape_factor * hydro * pow(f / n_to_n, 1.0 / (law.power + 1.0));
  }

  auto guess = lambda_q > lambda_h ? lambda_q : lambda_h;
  const auto minimum = minimumPermissibleValue(effective_trial_stress);
  const auto maximum = maximumPermissibleValue(effective_trial_stress);
  if (guess < minimum)
    guess = minimum;
  if (guess > maximum)
    guess = maximum;

  return guess;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::maximumPermissibleValue(
    const GenericReal<is_ad> & effective_trial_stress) const
{
  return gaugeStressScale(effective_trial_stress, _gauge_solve_state.effective_hydro_stress) *
         _maximum_gauge_ratio;
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
  auto minimum = effective_trial_stress;
  const auto positive_floor =
      gaugeStressScale(effective_trial_stress, _gauge_solve_state.effective_hydro_stress) /
      _maximum_gauge_ratio;
  if (positive_floor > minimum)
    minimum = positive_floor;

  return minimum;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeGaugeResidual(
    const GenericReal<is_ad> & equiv_stress,
    const GenericReal<is_ad> & trial_gauge,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & porosity,
    const CreepLaw & law,
    GenericReal<is_ad> & derivative) const
{
  using std::abs;
  using std::cosh;
  using std::sinh;

  const auto M = abs(effective_hydro_stress) / trial_gauge;
  const auto dM_dtrial_gauge = -M / trial_gauge;
  const auto residual_left = Utility::pow<2>(equiv_stress / trial_gauge);
  const auto dresidual_left_dtrial_gauge = -2.0 * residual_left / trial_gauge;

  auto residual = residual_left;
  derivative = dresidual_left_dtrial_gauge;

  if (_pore_shape == PoreShapeModel::SPHERICAL)
  {
    residual *= 1.0 + porosity / 1.5;
    derivative *= 1.0 + porosity / 1.5;
  }
  if (_model == ViscoplasticityModel::GTN)
  {
    residual += 2.0 * porosity * cosh(_pore_shape_factor * M) - 1.0 - Utility::pow<2>(porosity);
    derivative +=
        2.0 * porosity * sinh(_pore_shape_factor * M) * _pore_shape_factor * dM_dtrial_gauge;
  }
  else
  {
    const auto h = computeH(law.power, M);
    const auto dh_dM = computeH(law.power, M, true);
    residual +=
        porosity * (h + law.power_factor / h) - 1.0 - law.power_factor * Utility::pow<2>(porosity);
    const auto dresidual_dh = porosity * (1.0 - law.power_factor / Utility::pow<2>(h));
    derivative += dresidual_dh * dh_dM * dM_dtrial_gauge;
  }
  if (_verbose)
    Moose::out << "in computeResidual:\n"
               << "  position: " << _q_point[_qp]
               << " effective_hydro_stress: " << MetaPhysicL::raw_value(effective_hydro_stress)
               << " porosity: " << MetaPhysicL::raw_value(porosity)
               << " equiv_stress: " << MetaPhysicL::raw_value(equiv_stress)
               << " trial_gauge: " << MetaPhysicL::raw_value(trial_gauge) << " power: " << law.power
               << " M: " << MetaPhysicL::raw_value(M)
               << "\n  residual: " << MetaPhysicL::raw_value(residual)
               << " derivative: " << MetaPhysicL::raw_value(derivative) << std::endl;
  return residual;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeResidual(
    const GenericReal<is_ad> & equiv_stress, const GenericReal<is_ad> & trial_gauge)
{
  mooseAssert(_gauge_solve_state.law, "Gauge-stress solve does not have an active creep law.");
  return computeGaugeResidual(equiv_stress,
                              trial_gauge,
                              _gauge_solve_state.effective_hydro_stress,
                              _gauge_solve_state.porosity,
                              *_gauge_solve_state.law,
                              _derivative);
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeH(const Real n,
                                                        const GenericReal<is_ad> & M,
                                                        const bool derivative) const
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
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LpsDerivatives
PorousViscoplasticityStressUpdateTempl<is_ad>::computeLpsDerivatives(
    const GenericReal<is_ad> & gauge_stress,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & equiv_stress,
    const GenericReal<is_ad> & porosity,
    const CreepLaw & law) const
{
  using std::abs;
  using std::pow;

  mooseAssert(_model == ViscoplasticityModel::LPS,
              "LPS derivatives may only be evaluated for the LPS model.");

  LpsDerivatives d;

  const auto n = law.power;
  const auto alpha = law.power_factor;
  const auto beta = _pore_shape_factor;

  const auto lambda = gauge_stress;
  const auto q = equiv_stress;
  const auto f = porosity;
  const auto abs_p = abs(effective_hydro_stress);
  const auto M = abs_p / lambda;

  const auto sign_p = effective_hydro_stress > 0.0
                          ? Real(1.0)
                          : (effective_hydro_stress < 0.0 ? Real(-1.0) : Real(0.0));

  const auto exponent = (n + 1.0) / n;
  const auto mod = pow(beta * M, exponent);
  const auto y = 1.0 + mod / n;
  const auto h = pow(y, n);

  auto dh_dM = GenericReal<is_ad>(0.0);
  auto d2h_dM2 = GenericReal<is_ad>(0.0);

  if (M > 0.0)
  {
    const auto dmod_dM = exponent * mod / M;
    const auto d2mod_dM2 = exponent * (exponent - 1.0) * mod / Utility::pow<2>(M);

    dh_dM = dmod_dM * pow(y, n - 1.0);
    d2h_dM2 =
        (n - 1.0) / n * Utility::pow<2>(dmod_dM) * pow(y, n - 2.0) + d2mod_dM2 * pow(y, n - 1.0);
  }
  else if (MooseUtils::absoluteFuzzyEqual(n, 1.0))
    d2h_dM2 = 2.0 * beta * beta;
  else
  {
    /*
     * For n>1, the LPS H(M) is C1 but not C2 at M=0 because (n+1)/n is between
     * one and two. Use the zero-curvature semismooth choice at that isolated point.
     */
    d2h_dM2 = 0.0;
  }

  const auto Z = h + alpha / h;
  const auto dZ_dM = dh_dM * (1.0 - alpha / Utility::pow<2>(h));
  const auto d2Z_dM2 = d2h_dM2 * (1.0 - alpha / Utility::pow<2>(h)) +
                       2.0 * alpha * Utility::pow<2>(dh_dM) / Utility::pow<3>(h);

  const auto spherical = _pore_shape == PoreShapeModel::SPHERICAL;
  const auto A = spherical ? 1.0 + 2.0 * f / 3.0 : GenericReal<is_ad>(1.0);
  const auto dA_df = spherical ? Real(2.0 / 3.0) : Real(0.0);
  const auto q2_over_lambda2 = Utility::pow<2>(q / lambda);
  const auto left = A * q2_over_lambda2;

  const auto M_lambda = -M / lambda;
  const auto M_p = sign_p / lambda;
  const auto M_lambdalambda = 2.0 * M / Utility::pow<2>(lambda);
  const auto M_lambdap = -sign_p / Utility::pow<2>(lambda);

  d.F_lambda = -2.0 * left / lambda + f * dZ_dM * M_lambda;
  d.F_p = f * dZ_dM * M_p;
  d.F_q = 2.0 * A * q / Utility::pow<2>(lambda);
  d.F_f = dA_df * q2_over_lambda2 + Z - 2.0 * alpha * f;

  d.F_lambdalambda = 6.0 * left / Utility::pow<2>(lambda) +
                     f * (d2Z_dM2 * Utility::pow<2>(M_lambda) + dZ_dM * M_lambdalambda);

  d.F_lambdap = f * (d2Z_dM2 * M_lambda * M_p + dZ_dM * M_lambdap);
  d.F_lambdaq = -4.0 * A * q / Utility::pow<3>(lambda);
  d.F_lambdaf = -2.0 * dA_df * Utility::pow<2>(q) / Utility::pow<3>(lambda) + dZ_dM * M_lambda;

  d.F_pp = f * d2Z_dM2 * Utility::pow<2>(M_p);
  d.F_pf = dZ_dM * M_p;
  return d;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LpsMechanismResponse
PorousViscoplasticityStressUpdateTempl<is_ad>::evaluateLpsMechanismResponse(
    const std::size_t law_index,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & equiv_stress,
    const GenericRankTwoTensor<is_ad> & dev_direction,
    const GenericReal<is_ad> & porosity)
{
  using std::abs;

  mooseAssert(_model == ViscoplasticityModel::LPS,
              "The analytical mechanism response is available only for the LPS model.");
  mooseAssert(law_index < _creep_laws.size(), "Creep-law index is out of range.");

  auto response = LpsMechanismResponse{};
  response.inelastic_strain_increment.zero();
  for (auto & derivative : response.dinelastic_dx)
    derivative.zero();

  response.coefficient = creepCoefficient(law_index);
  if (MetaPhysicL::raw_value(response.coefficient) == 0.0)
    return response;

  response.active = true;
  const auto & law = _creep_laws[law_index];
  computeGaugeStress(response.gauge_stress, equiv_stress, effective_hydro_stress, porosity, law);

  const auto lps = computeLpsDerivatives(
      response.gauge_stress, effective_hydro_stress, equiv_stress, porosity, law);
  const auto F_lambda_raw = MetaPhysicL::raw_value(lps.F_lambda);
  const auto scaled_F_lambda_raw = MetaPhysicL::raw_value(response.gauge_stress * lps.F_lambda);
  if (!std::isfinite(F_lambda_raw) || !std::isfinite(scaled_F_lambda_raw) ||
      abs(scaled_F_lambda_raw) < 1.0e-12)
    mooseException("In ",
                   _name,
                   ": singular analytical LPS derivative Lambda*dF/dLambda = ",
                   scaled_F_lambda_raw,
                   " for creep law ",
                   law_index,
                   " with power = ",
                   law.power,
                   ".");

  response.F_lambda = lps.F_lambda;
  response.dgauge_dx = {-lps.F_p / lps.F_lambda, -lps.F_q / lps.F_lambda, -lps.F_f / lps.F_lambda};

  const auto dev_stress = dev_direction * equiv_stress;
  const auto spherical = _pore_shape == PoreShapeModel::SPHERICAL;
  const auto A = spherical ? 1.0 + 2.0 * porosity / 3.0 : GenericReal<is_ad>(1.0);
  const auto dA_df = spherical ? Real(2.0 / 3.0) : Real(0.0);

  const std::array<GenericReal<is_ad>, 3> p_x = {
      GenericReal<is_ad>(1.0), GenericReal<is_ad>(0.0), GenericReal<is_ad>(0.0)};
  const std::array<GenericReal<is_ad>, 3> q_x = {
      GenericReal<is_ad>(0.0), GenericReal<is_ad>(1.0), GenericReal<is_ad>(0.0)};
  const std::array<GenericReal<is_ad>, 3> f_x = {
      GenericReal<is_ad>(0.0), GenericReal<is_ad>(0.0), GenericReal<is_ad>(1.0)};

  const auto dev_factor = 3.0 * A / Utility::pow<2>(response.gauge_stress);
  const auto B = _identity_two * (lps.F_p / 3.0) + dev_stress * dev_factor;
  response.creep_rate = computeCreepRate(law, response.coefficient, response.gauge_stress);
  const auto W = -this->constitutiveTimeStep() * response.creep_rate / lps.F_lambda;

  response.inelastic_strain_increment = B * W;
  response.effective_inelastic_strain_increment =
      response.creep_rate * this->constitutiveTimeStep();

  const auto dev_factor_lambda = -2.0 * dev_factor / response.gauge_stress;
  const auto dev_factor_f = 3.0 * dA_df / Utility::pow<2>(response.gauge_stress);

  for (auto column = 0u; column < 3; ++column)
  {
    const auto dFp_dx = lps.F_lambdap * response.dgauge_dx[column] + lps.F_pp * p_x[column] +
                        lps.F_pf * f_x[column];
    const auto dFlambda_dx = lps.F_lambdalambda * response.dgauge_dx[column] +
                             lps.F_lambdap * p_x[column] + lps.F_lambdaq * q_x[column] +
                             lps.F_lambdaf * f_x[column];
    const auto ddev_factor_dx =
        dev_factor_lambda * response.dgauge_dx[column] + dev_factor_f * f_x[column];
    const auto ddev_stress_dx = dev_direction * q_x[column];
    const auto dB_dx =
        _identity_two * (dFp_dx / 3.0) + dev_stress * ddev_factor_dx + ddev_stress_dx * dev_factor;
    const auto dW_dx = W * (law.power * response.dgauge_dx[column] / response.gauge_stress -
                            dFlambda_dx / lps.F_lambda);

    response.dinelastic_dx[column] = B * dW_dx + dB_dx * W;
  }

  return response;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LpsCreepResponse
PorousViscoplasticityStressUpdateTempl<is_ad>::evaluateLpsCreepResponse(
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & equiv_stress,
    const GenericRankTwoTensor<is_ad> & dev_direction,
    const GenericReal<is_ad> & porosity)
{
  mooseAssert(_model == ViscoplasticityModel::LPS,
              "The summed analytical creep response is available only for the LPS model.");

  auto response = LpsCreepResponse{};
  response.inelastic_strain_increment.zero();
  for (auto & derivative : response.dinelastic_dx)
    derivative.zero();

  auto primary_set = false;
  for (auto law_index = std::size_t{0}; law_index < _creep_laws.size(); ++law_index)
  {
    const auto mechanism = evaluateLpsMechanismResponse(
        law_index, effective_hydro_stress, equiv_stress, dev_direction, porosity);

    if (!mechanism.active)
      continue;
    if (!primary_set)
    {
      response.primary_gauge_stress = mechanism.gauge_stress;
      primary_set = true;
    }

    response.inelastic_strain_increment += mechanism.inelastic_strain_increment;
    response.effective_inelastic_strain_increment += mechanism.effective_inelastic_strain_increment;
    for (auto column = 0u; column < 3; ++column)
      response.dinelastic_dx[column] += mechanism.dinelastic_dx[column];
  }

  return response;
}

template <bool is_ad>
GenericRankTwoTensor<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeDGaugeDSigma(
    const GenericReal<is_ad> & gauge_stress,
    const GenericReal<is_ad> & equiv_stress,
    const GenericRankTwoTensor<is_ad> & dev_stress,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & porosity,
    const CreepLaw & law) const
{
  using std::abs;
  using std::sinh;

  auto dresidual_dequiv_stress_dequiv_stress_dsigma =
      3.0 * dev_stress / Utility::pow<2>(gauge_stress);
  if (_pore_shape == PoreShapeModel::SPHERICAL)
    dresidual_dequiv_stress_dequiv_stress_dsigma *= 1.0 + 2.0 * porosity / 3.0;

  if (_model == ViscoplasticityModel::LPS)
  {
    const auto lps =
        computeLpsDerivatives(gauge_stress, effective_hydro_stress, equiv_stress, porosity, law);
    const auto dresidual_dsigma =
        lps.F_p * _dhydro_stress_dsigma + dresidual_dequiv_stress_dequiv_stress_dsigma;
    return -dresidual_dsigma / lps.F_lambda;
  }

  const auto M = abs(effective_hydro_stress) / gauge_stress;
  auto dresidual_deffective_hydro_stress = GenericReal<is_ad>(0.0);
  if (effective_hydro_stress != 0.0)
  {
    const auto dM_deffective_hydro_stress = M / effective_hydro_stress;
    dresidual_deffective_hydro_stress = 2.0 * porosity * sinh(_pore_shape_factor * M) *
                                        _pore_shape_factor * dM_deffective_hydro_stress;
  }

  const auto dresidual_dsigma = dresidual_deffective_hydro_stress * _dhydro_stress_dsigma +
                                dresidual_dequiv_stress_dequiv_stress_dsigma;

  // Re-evaluate the residual at the converged gauge stress to obtain dF/dLambda for GTN branches
  // that obtain Lambda analytically instead of through returnMappingSolve().
  auto dresidual_dgauge = GenericReal<is_ad>(0.0);
  computeGaugeResidual(
      equiv_stress, gauge_stress, effective_hydro_stress, porosity, law, dresidual_dgauge);

  return -dresidual_dsigma / dresidual_dgauge;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::computeGaugeStress(
    GenericReal<is_ad> & gauge_stress,
    const GenericReal<is_ad> & equiv_stress,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & porosity,
    const CreepLaw & law)
{
  using std::sqrt;

  if (porosity == 0.0)
    gauge_stress = equiv_stress;
  else if (effective_hydro_stress == 0.0)
  {
    const auto A =
        _pore_shape == PoreShapeModel::SPHERICAL ? 1.0 + 2.0 * porosity / 3.0 : Real(1.0);
    gauge_stress = equiv_stress * sqrt(A) /
                   sqrt(1.0 - (1.0 + law.power_factor) * porosity +
                        law.power_factor * Utility::pow<2>(porosity));
  }
  else
  {
    _gauge_solve_state.effective_hydro_stress = effective_hydro_stress;
    _gauge_solve_state.porosity = porosity;
    _gauge_solve_state.law = &law;
    this->returnMappingSolve(equiv_stress, gauge_stress, _console);
  }
  mooseAssert(gauge_stress >= equiv_stress,
              "Gauge stress calculated in inner Newton solve is less than the equivalent stress.");
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::computeInelasticStrainIncrement(
    GenericReal<is_ad> & total_creep_rate,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericReal<is_ad> & equiv_stress,
    const GenericRankTwoTensor<is_ad> & dev_stress,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & porosity)
{
  total_creep_rate = 0.0;
  inelastic_strain_increment.zero();
  _gauge_stress[_qp] = 0.0;
  auto primary_set = false;

  for (auto law_index = std::size_t{0}; law_index < _creep_laws.size(); ++law_index)
  {
    const auto coefficient = creepCoefficient(law_index);
    const auto zero_coefficient = MetaPhysicL::raw_value(coefficient) == 0.0;
    // Preserve the legacy single-law gauge diagnostic when C = 0. In a multi-law model, an
    // exactly zero coefficient denotes an inactive mechanism and its per-law gauge remains zero.
    if (zero_coefficient && _creep_laws.size() > 1)
    {
      setGaugeStress(law_index, GenericReal<is_ad>(0.0));
      continue;
    }

    const auto & law = _creep_laws[law_index];
    GenericReal<is_ad> gauge_stress;
    computeGaugeStress(gauge_stress, equiv_stress, effective_hydro_stress, porosity, law);
    setGaugeStress(law_index, gauge_stress);
    if (!primary_set)
    {
      _gauge_stress[_qp] = gauge_stress;
      primary_set = true;
    }
    if (zero_coefficient)
      continue;

    const auto creep_rate = computeCreepRate(law, coefficient, gauge_stress);
    total_creep_rate += creep_rate;
    inelastic_strain_increment +=
        this->constitutiveTimeStep() * creep_rate *
        computeDGaugeDSigma(
            gauge_stress, equiv_stress, dev_stress, effective_hydro_stress, porosity, law);
  }
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
    const GenericReal<is_ad> & /*effective_trial_stress*/,
    const GenericReal<is_ad> & /*gauge_stress*/)
{
  // The porous gauge residual is dimensionless, so its relative convergence reference must also be
  // dimensionless. Using Lambda here would make the convergence criterion depend on stress units.
  return 1.0;
}

template class PorousViscoplasticityStressUpdateTempl<false>;
template class PorousViscoplasticityStressUpdateTempl<true>;
