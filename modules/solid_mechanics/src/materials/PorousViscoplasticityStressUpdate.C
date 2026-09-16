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

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{
template <typename T, std::size_t N>
using FixedMatrix = std::array<std::array<T, N>, N>;

template <typename T, std::size_t N>
using FixedVector = std::array<T, N>;

/**
 * Solve the small fixed-size linear systems used by the local constitutive update.
 *
 * The 2x2 branch intentionally retains the determinant-based singularity check and closed-form
 * solution used by the reduced mechanical solve. The 3x3 branch retains the partial-pivot
 * Gauss-Jordan algorithm and pivot threshold used by the coupled Newton solve.
 */
template <std::size_t N, typename MatrixValue, typename VectorValue>
bool
solveLinearSystem(const FixedMatrix<MatrixValue, N> & input,
                  const FixedVector<VectorValue, N> & rhs,
                  FixedVector<VectorValue, N> & solution)
{
  static_assert(N == 2 || N == 3, "Only the local 2x2 and 3x3 systems are supported.");

  for (auto row = std::size_t{0}; row < N; ++row)
  {
    if (!std::isfinite(MetaPhysicL::raw_value(rhs[row])))
      return false;

    for (auto column = std::size_t{0}; column < N; ++column)
      if (!std::isfinite(MetaPhysicL::raw_value(input[row][column])))
        return false;
  }

  if constexpr (N == 2)
  {
    using std::abs;
    using std::max;

    const auto determinant = input[0][0] * input[1][1] - input[0][1] * input[1][0];
    const auto j00 = MetaPhysicL::raw_value(input[0][0]);
    const auto j01 = MetaPhysicL::raw_value(input[0][1]);
    const auto j10 = MetaPhysicL::raw_value(input[1][0]);
    const auto j11 = MetaPhysicL::raw_value(input[1][1]);
    const auto determinant_raw = MetaPhysicL::raw_value(determinant);
    const auto jacobian_scale = max({abs(j00), abs(j01), abs(j10), abs(j11), 1.0});

    if (!std::isfinite(determinant_raw) ||
        abs(determinant_raw) <= 1.0e-14 * jacobian_scale * jacobian_scale)
      return false;

    solution[0] = (input[1][1] * rhs[0] - input[0][1] * rhs[1]) / determinant;
    solution[1] = (-input[1][0] * rhs[0] + input[0][0] * rhs[1]) / determinant;
    return std::isfinite(MetaPhysicL::raw_value(solution[0])) &&
           std::isfinite(MetaPhysicL::raw_value(solution[1]));
  }
  else
  {
    auto matrix = input;
    solution = rhs;

    for (auto column = std::size_t{0}; column < N; ++column)
    {
      auto pivot = column;
      auto pivot_abs = std::abs(MetaPhysicL::raw_value(matrix[column][column]));

      for (auto row = column + 1; row < N; ++row)
      {
        const auto candidate = std::abs(MetaPhysicL::raw_value(matrix[row][column]));
        if (candidate > pivot_abs)
        {
          pivot = row;
          pivot_abs = candidate;
        }
      }

      if (!std::isfinite(pivot_abs) || pivot_abs < 1.0e-14)
        return false;

      if (pivot != column)
      {
        std::swap(matrix[pivot], matrix[column]);
        std::swap(solution[pivot], solution[column]);
      }

      const auto diagonal = matrix[column][column];
      for (auto j = std::size_t{0}; j < N; ++j)
        matrix[column][j] /= diagonal;
      solution[column] /= diagonal;

      for (auto row = std::size_t{0}; row < N; ++row)
      {
        if (row == column)
          continue;

        const auto factor = matrix[row][column];
        for (auto j = std::size_t{0}; j < N; ++j)
          matrix[row][j] -= factor * matrix[column][j];
        solution[row] -= factor * solution[column];
      }
    }

    for (const auto & value : solution)
      if (!std::isfinite(MetaPhysicL::raw_value(value)))
        return false;

    return true;
  }
}

/**
 * Return the mechanically scaled residual norm used only for convergence decisions.
 *
 * The p and q scales used by the Newton system also contain elastic-modulus times
 * max_inelastic_increment terms for numerical conditioning. Those terms must not loosen the
 * physical convergence test, because max_inelastic_increment can be many orders of magnitude
 * larger than the actual constitutive increment.
 */
template <typename ResidualValue, std::size_t N, typename PValue, typename QValue>
Real
physicalMechanicalResidualNorm(const std::array<ResidualValue, N> & residual,
                               const PValue & p_trial,
                               const QValue & q_trial,
                               const Real minimum_stress_magnitude)
{
  static_assert(N >= 2, "The mechanical residual requires p and q components.");

  using std::abs;
  using std::max;

  const auto p_convergence_scale =
      max({abs(MetaPhysicL::raw_value(p_trial)), minimum_stress_magnitude, 1.0});
  const auto q_convergence_scale =
      max({abs(MetaPhysicL::raw_value(q_trial)), minimum_stress_magnitude, 1.0});
  const auto scaled_p = MetaPhysicL::raw_value(residual[0]) / p_convergence_scale;
  const auto scaled_q = MetaPhysicL::raw_value(residual[1]) / q_convergence_scale;

  if (!std::isfinite(scaled_p) || !std::isfinite(scaled_q))
    return std::numeric_limits<Real>::infinity();

  return std::hypot(scaled_p, scaled_q);
}

} // namespace

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
      "Computes the spherical LPS porous viscoplastic response by solving matrix hydrostatic "
      "stress, equivalent stress, and porosity simultaneously. One or more Norton creep laws may "
      "be supplied; each exponent is evaluated on its own exponent-dependent LPS gauge surface. "
      "Derived models may provide a porosity-dependent pressure closure.");
  MooseEnum legacy_viscoplasticity_model("LPS GTN", "LPS");
  params.addDeprecatedParam<MooseEnum>(
      "viscoplasticity_model",
      legacy_viscoplasticity_model,
      "Legacy selector for the porous viscoplastic formulation.",
      "The GTN formulation has been removed. PorousViscoplasticityStressUpdate now uses the LPS "
      "formulation exclusively; remove this parameter from the input.");
  MooseEnum legacy_pore_shape_model("spherical cylindrical", "spherical");
  params.addDeprecatedParam<MooseEnum>(
      "pore_shape_model",
      legacy_pore_shape_model,
      "Legacy selector for the pore geometry.",
      "The cylindrical-pore formulation has been removed. PorousViscoplasticityStressUpdate now "
      "assumes spherical pores; remove this parameter from the input.");
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

  params.addRangeCheckedParam<Real>("minimum_porosity",
                                    0.0,
                                    "minimum_porosity >= 0.0 & minimum_porosity < 1.0",
                                    "Lower bound for the coupled local porosity active set.");
  params.setDocUnit("minimum_porosity", "unitless");

  params.addRangeCheckedParam<Real>(
      "porosity_bound_tolerance",
      1.0e-12,
      "porosity_bound_tolerance > 0.0",
      "Absolute tolerance used to activate and release the local porosity lower bound.");

  params.addRangeCheckedParam<Real>(
      "local_newton_tolerance",
      1.0e-8,
      "local_newton_tolerance > 0.0",
      "Dimensionless convergence tolerance on the scaled coupled (p,q,f) residual norm.");

  params.addRangeCheckedParam<Real>(
      "local_newton_stagnation_tolerance",
      1.0e-7,
      "local_newton_stagnation_tolerance > 0.0",
      "Near-converged residual norm below which a stalled local line search is accepted.");

  params.addRangeCheckedParam<unsigned int>(
      "local_newton_max_iterations",
      30,
      "local_newton_max_iterations > 0",
      "Maximum coupled (p,q,f) Newton iterations in one local constitutive substep.");

  params.addRangeCheckedParam<Real>(
      "local_newton_relaxation",
      1.0,
      "local_newton_relaxation > 0.0 & local_newton_relaxation <= 1.0",
      "Initial damping applied to the coupled local Newton correction.");

  params.addRangeCheckedParam<unsigned int>(
      "local_newton_max_backtracks",
      12,
      "local_newton_max_backtracks > 0",
      "Maximum number of line-search halvings for one coupled local Newton correction.");

  params.addRangeCheckedParam<Real>(
      "local_porosity_scale_floor",
      1.0e-6,
      "local_porosity_scale_floor > 0.0",
      "Minimum physical scale used to certify convergence of the local porosity residual. The "
      "Newton variable and line-search merit use a separate characteristic porosity-change scale "
      "so conditioning can be improved without loosening the accepted constitutive tolerance.");

  params.addRangeCheckedParam<unsigned int>(
      "reduced_porosity_max_probes",
      20,
      "reduced_porosity_max_probes > 0",
      "Maximum fixed-porosity mechanical solves used while bracketing the reduced porosity root.");

  params.addRangeCheckedParam<Real>(
      "reduced_porosity_probe_growth",
      8.0,
      "reduced_porosity_probe_growth > 1.0",
      "Trust-region growth factor applied to distance from the porosity floor during reduced-root "
      "bracket discovery.");

  params.addRangeCheckedParam<unsigned int>(
      "reduced_porosity_root_max_iterations",
      64,
      "reduced_porosity_root_max_iterations > 0",
      "Maximum hybrid Newton/bisection iterations after a sign-changing reduced-porosity bracket "
      "has been found.");

  params.addParamNamesToGroup(
      "verbose maximum_gauge_ratio maximum_stress_magnitude use_substepping "
      "substep_strain_tolerance adaptive_substepping maximum_number_substeps minimum_porosity "
      "porosity_bound_tolerance local_newton_tolerance local_newton_stagnation_tolerance "
      "local_newton_max_iterations local_newton_relaxation local_newton_max_backtracks "
      "local_porosity_scale_floor reduced_porosity_max_probes reduced_porosity_probe_growth "
      "reduced_porosity_root_max_iterations",
      "Advanced");

  return params;
}

template <bool is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::PorousViscoplasticityStressUpdateTempl(
    const InputParameters & parameters)
  : ViscoplasticityStressUpdateBaseTempl<is_ad>(parameters),
    SingleVariableReturnMappingSolutionTempl<is_ad>(parameters),
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
    _hydro_stress(0.0),
    _identity_two(RankTwoTensor::initIdentity),
    _derivative(0.0),
    _compute_consistent_tangent(false),
    _last_consistent_tangent(RankFourTensor::initIdentityFour),
    _minimum_porosity(this->template getParam<Real>("minimum_porosity")),
    _porosity_bound_tolerance(this->template getParam<Real>("porosity_bound_tolerance")),
    _local_newton_tolerance(this->template getParam<Real>("local_newton_tolerance")),
    _local_newton_stagnation_tolerance(
        this->template getParam<Real>("local_newton_stagnation_tolerance")),
    _local_newton_max_iterations(
        this->template getParam<unsigned int>("local_newton_max_iterations")),
    _local_newton_relaxation(this->template getParam<Real>("local_newton_relaxation")),
    _local_newton_max_backtracks(
        this->template getParam<unsigned int>("local_newton_max_backtracks")),
    _local_porosity_scale_floor(this->template getParam<Real>("local_porosity_scale_floor")),
    _reduced_porosity_max_probes(
        this->template getParam<unsigned int>("reduced_porosity_max_probes")),
    _reduced_porosity_probe_growth(this->template getParam<Real>("reduced_porosity_probe_growth")),
    _reduced_porosity_root_max_iterations(
        this->template getParam<unsigned int>("reduced_porosity_root_max_iterations"))
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

    const auto power_factor = (powers[i] - 1.0) / (powers[i] + 1.0);

    const auto * coefficient =
        &this->template getGenericMaterialProperty<Real, is_ad>(coefficient_names[i]);

    _creep_laws.push_back({coefficient, powers[i], power_factor});
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

  if (parameters.isParamSetByUser("viscoplasticity_model"))
    this->paramError(
        "viscoplasticity_model",
        "This parameter has been removed. PorousViscoplasticityStressUpdate now uses the LPS "
        "formulation exclusively. Remove viscoplasticity_model from the input.");

  if (parameters.isParamSetByUser("pore_shape_model"))
    this->paramError(
        "pore_shape_model",
        "This parameter has been removed. PorousViscoplasticityStressUpdate now assumes spherical "
        "pores. Remove pore_shape_model from the input.");

  if (_local_newton_stagnation_tolerance < _local_newton_tolerance)
    this->paramError("local_newton_stagnation_tolerance",
                     "local_newton_stagnation_tolerance must be greater than or equal to "
                     "local_newton_tolerance.");
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
      gauge_stress = computeGaugeStress(
          equiv_stress, effective_hydro_stress, porosity, _creep_laws[law_index]);
    setGaugeStress(law_index, gauge_stress);
    if (!primary_set)
    {
      _gauge_stress[_qp] = gauge_stress;
      primary_set = true;
    }
  }
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
typename PorousViscoplasticityStressUpdateTempl<is_ad>::HydrostaticStressState
PorousViscoplasticityStressUpdateTempl<is_ad>::evaluateHydrostaticStress(
    const GenericReal<is_ad> & matrix_hydro_stress, const GenericReal<is_ad> & /*porosity*/) const
{
  return {effectiveHydroStress(matrix_hydro_stress), 0.0};
}

template <bool is_ad>
TangentCalculationMethod
PorousViscoplasticityStressUpdateTempl<is_ad>::getTangentCalculationMethod()
{
  if constexpr (is_ad)
  {
    mooseError("getTangentCalculationMethod called: no tangent moduli calculation is needed "
               "while using AD");
    return TangentCalculationMethod::ELASTIC;
  }
  else
  {
    // The one-step tangent is exact. A FULL tangent for local substepping would also need to
    // propagate mechanical, porosity, and derived-model state sensitivities through every substep.
    return substeppingCapabilityRequested() ? TangentCalculationMethod::ELASTIC
                                            : TangentCalculationMethod::FULL;
  }
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
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::equivalentStress(
    const GenericRankTwoTensor<is_ad> & dev_stress)
{
  using std::sqrt;

  const auto squared = dev_stress.doubleContraction(dev_stress);
  return squared == 0.0 ? GenericReal<is_ad>(0.0) : sqrt(1.5 * squared);
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::ConstitutiveStateSnapshot
PorousViscoplasticityStressUpdateTempl<is_ad>::captureConstitutiveState(
    const GenericRankTwoTensor<is_ad> & strain_increment,
    const GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericRankTwoTensor<is_ad> & stress) const
{
  auto snapshot = ConstitutiveStateSnapshot{};
  snapshot.strain_increment = strain_increment;
  snapshot.inelastic_strain_increment = inelastic_strain_increment;
  snapshot.stress = stress;
  snapshot.intermediate_porosity = _intermediate_porosity;
  snapshot.hydro_stress = _hydro_stress;
  snapshot.gauge_stress = _gauge_stress[_qp];
  snapshot.gauge_stresses.reserve(_gauge_stress_laws.size());
  for (const auto * gauge_stress : _gauge_stress_laws)
    snapshot.gauge_stresses.push_back((*gauge_stress)[_qp]);

  return snapshot;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::restoreConstitutiveState(
    const ConstitutiveStateSnapshot & snapshot,
    GenericRankTwoTensor<is_ad> & strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    GenericRankTwoTensor<is_ad> & stress)
{
  mooseAssert(snapshot.gauge_stresses.size() == _gauge_stress_laws.size(),
              "Gauge-stress snapshot size must match the configured creep laws.");

  this->resetConstitutiveTimeStep();
  strain_increment = snapshot.strain_increment;
  inelastic_strain_increment = snapshot.inelastic_strain_increment;
  stress = snapshot.stress;
  resetIncrementalMaterialProperties();
  _intermediate_porosity = snapshot.intermediate_porosity;
  _hydro_stress = snapshot.hydro_stress;
  _gauge_stress[_qp] = snapshot.gauge_stress;
  for (auto law_index = std::size_t{0}; law_index < _gauge_stress_laws.size(); ++law_index)
    (*_gauge_stress_laws[law_index])[_qp] = snapshot.gauge_stresses[law_index];
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
    const bool compute_full_tangent_operator,
    RankFourTensor & tangent_operator)
{
  const auto previous_tangent_request = _compute_consistent_tangent;
  const auto tangent_requested =
      !is_ad && compute_full_tangent_operator && !substeppingCapabilityRequested();
  _compute_consistent_tangent = tangent_requested;

  // _dt aliases FEProblem::dt(), so never use it as constitutive scratch storage.
  this->resetConstitutiveTimeStep();
  const auto snapshot =
      captureConstitutiveState(elastic_strain_increment, inelastic_strain_increment, stress);

  try
  {
    this->updateIntermediatePorosity(elastic_strain_increment);
    resetIncrementalMaterialProperties();
    updateStateSubstepInternal(elastic_strain_increment,
                               inelastic_strain_increment,
                               stress,
                               elasticity_tensor,
                               elastic_strain_old,
                               1);

    if constexpr (!is_ad)
      if (tangent_requested)
        tangent_operator = _last_consistent_tangent;
  }
  catch (...)
  {
    restoreConstitutiveState(
        snapshot, elastic_strain_increment, inelastic_strain_increment, stress);
    _compute_consistent_tangent = previous_tangent_request;
    throw;
  }

  _compute_consistent_tangent = previous_tangent_request;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint
PorousViscoplasticityStressUpdateTempl<is_ad>::evaluateLocalPoint(
    const LocalCoordinates & coordinates,
    const LocalSolveContext & context,
    const PorosityBranch porosity_branch)
{
  const auto & p = coordinates.p;
  const auto & q = coordinates.q;
  const auto & f = coordinates.f;
  const auto floor_active = porosity_branch == PorosityBranch::FLOOR;

  auto point = LocalPoint{};
  point.p = p;
  point.q = q;
  point.f = f;
  point.porosity_branch = porosity_branch;
  point.inelastic_strain_increment.zero();

  const auto has_deviatoric_direction =
      context.trial_equiv_stress > this->_minimum_stress_magnitude;

  auto dev_direction = GenericRankTwoTensor<is_ad>();
  dev_direction.zero();

  if (has_deviatoric_direction)
    dev_direction = context.trial_dev_stress / context.trial_equiv_stress;

  const auto q_flow = has_deviatoric_direction ? q : GenericReal<is_ad>(0.0);
  const auto dev_stress = dev_direction * q_flow;

  // Evaluate the complete hydrostatic driving state once so every creep mechanism sees the same
  // pressure closure and porosity derivative at this local (p,q,f) point.
  point.hydrostatic_stress = evaluateHydrostaticStress(p, f);
  const auto & hydrostatic_stress = point.hydrostatic_stress;

  if (this->gaugeStressScale(q_flow, hydrostatic_stress.effective_hydro_stress) >
      this->_maximum_stress_magnitude)
    mooseException("In ",
                   this->_name,
                   ": local coupled solve exceeded maximum_stress_magnitude at p = ",
                   MetaPhysicL::raw_value(p),
                   ", q = ",
                   MetaPhysicL::raw_value(q_flow),
                   ", f = ",
                   MetaPhysicL::raw_value(f),
                   ".");

  /*
   * If the model-specific pressure closure gives zero effective hydrostatic drive and there is no
   * deviatoric drive either, all creep mechanisms are inactive.
   */
  if (!this->hasViscoplasticDrive(q_flow, hydrostatic_stress.effective_hydro_stress, f))
  {
    const auto trial_stress = context.elasticity_tensor *
                              (context.elastic_strain_old + context.trial_elastic_strain_increment);
    const auto trial_p = trial_stress.trace() / 3.0;

    point.residual[P_INDEX] = p - trial_p;
    point.residual[Q_INDEX] = has_deviatoric_direction ? q - context.trial_equiv_stress : q;
    point.residual[F_INDEX] = floor_active ? f - _minimum_porosity : f - context.porosity_begin;

    point.jacobian[P_INDEX][P_INDEX] = 1.0;
    point.jacobian[Q_INDEX][Q_INDEX] = 1.0;
    point.jacobian[F_INDEX][F_INDEX] = 1.0;
    return point;
  }

  /*
   * Derived models may make p_eff depend on f. evaluateHydrostaticStress() supplies the
   * complete hydrostatic driving stress and its explicit porosity derivative while the local
   * nonlinear topology remains fixed.
   */
  const auto deffective_hydro_df = hydrostatic_stress.deffective_hydro_df;

  /*
   * The generic MOOSE porous-LPS base evaluates all exponent-dependent gauge surfaces, sums the
   * creep increments, and returns exact partial derivatives with respect to p_eff, q, and f.
   * Model-specific pore physics enters here only through dp_eff/df.
   */
  const auto creep_response = this->evaluateLpsCreepResponse(
      hydrostatic_stress.effective_hydro_stress, q_flow, dev_direction, f);

  point.effective_inelastic_strain_increment = creep_response.effective_inelastic_strain_increment;

  const auto raw_inelastic_strain_increment = creep_response.inelastic_strain_increment;
  auto raw_dinelastic_dx = std::array<GenericRankTwoTensor<is_ad>, LOCAL_SYSTEM_SIZE>{
      creep_response.dinelastic_deffective_hydro_stress,
      creep_response.dinelastic_dequiv_stress,
      creep_response.dinelastic_dporosity +
          creep_response.dinelastic_deffective_hydro_stress * deffective_hydro_df};

  point.inelastic_strain_increment = raw_inelastic_strain_increment;

  /*
   * On the lower-bound active branch, constrain only the total volumetric increment. The summed
   * deviatoric creep from every mechanism remains active.
   */
  if (floor_active)
  {
    const auto porosity_factor = 1.0 - this->_porosity_old[this->_qp];

    if (porosity_factor <= 0.0)
      mooseException("In ", this->_name, ": invalid porosity factor at the lower bound.");

    const auto allowed_trace = (_minimum_porosity - context.porosity_begin) / porosity_factor;
    point.inelastic_strain_increment =
        raw_inelastic_strain_increment.deviatoric() + this->_identity_two * (allowed_trace / 3.0);
  }

  const auto stress_calculated =
      context.elasticity_tensor *
      (context.elastic_strain_old + context.trial_elastic_strain_increment -
       point.inelastic_strain_increment);
  const auto p_calculated = stress_calculated.trace() / 3.0;
  const auto dev_stress_calculated = stress_calculated.deviatoric();

  /* For s = q n with n:n = 2/3, q = (3/2) n:s. */
  const auto q_calculated = has_deviatoric_direction
                                ? 1.5 * dev_direction.doubleContraction(dev_stress_calculated)
                                : GenericReal<is_ad>(0.0);

  point.residual[P_INDEX] = p - p_calculated;
  point.residual[Q_INDEX] = has_deviatoric_direction ? q - q_calculated : q;
  point.residual[F_INDEX] = floor_active ? f - _minimum_porosity
                                         : f - context.porosity_begin -
                                               (1.0 - this->_porosity_old[this->_qp]) *
                                                   point.inelastic_strain_increment.trace();

  for (auto column = 0u; column < LOCAL_SYSTEM_SIZE; ++column)
  {
    /* The active floor fixes the volumetric increment, leaving only its deviatoric derivative. */
    const auto dinelastic_dx =
        floor_active ? raw_dinelastic_dx[column].deviatoric() : raw_dinelastic_dx[column];
    const auto elastic_response = context.elasticity_tensor * dinelastic_dx;

    point.jacobian[P_INDEX][column] =
        (column == P_INDEX ? GenericReal<is_ad>(1.0) : GenericReal<is_ad>(0.0)) +
        elastic_response.trace() / 3.0;

    if (has_deviatoric_direction)
      point.jacobian[Q_INDEX][column] =
          (column == Q_INDEX ? GenericReal<is_ad>(1.0) : GenericReal<is_ad>(0.0)) +
          1.5 * dev_direction.doubleContraction(elastic_response.deviatoric());
    else
      point.jacobian[Q_INDEX][column] =
          column == Q_INDEX ? GenericReal<is_ad>(1.0) : GenericReal<is_ad>(0.0);

    if (floor_active)
      point.jacobian[F_INDEX][column] =
          column == F_INDEX ? GenericReal<is_ad>(1.0) : GenericReal<is_ad>(0.0);
    else
      point.jacobian[F_INDEX][column] =
          (column == F_INDEX ? GenericReal<is_ad>(1.0) : GenericReal<is_ad>(0.0)) -
          (1.0 - this->_porosity_old[this->_qp]) * dinelastic_dx.trace();
  }

  return point;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalResidual
PorousViscoplasticityStressUpdateTempl<is_ad>::scaledResidual(
    const LocalResidual & residual, const LocalSolveContext & context) const
{
  return {residual[P_INDEX] / context.p_scale,
          residual[Q_INDEX] / context.q_scale,
          residual[F_INDEX] / context.porosity_merit_scale};
}

template <bool is_ad>
Real
PorousViscoplasticityStressUpdateTempl<is_ad>::convergenceResidualNorm(
    const LocalResidual & residual, const LocalSolveContext & context) const
{
  return residualNorm({residual[P_INDEX] / context.p_scale,
                       residual[Q_INDEX] / context.q_scale,
                       residual[F_INDEX] / context.porosity_convergence_scale});
}

template <bool is_ad>
Real
PorousViscoplasticityStressUpdateTempl<is_ad>::residualNorm(const LocalResidual & residual,
                                                            const LocalResidualScope scope) const
{
  auto norm_squared = Real(0.0);
  const auto components = scope == LocalResidualScope::MECHANICAL ? 2u : 3u;
  for (auto component = 0u; component < components; ++component)
  {
    const auto raw = MetaPhysicL::raw_value(residual[component]);
    if (!std::isfinite(raw))
      return std::numeric_limits<Real>::infinity();

    norm_squared += raw * raw;
    if (!std::isfinite(norm_squared))
      return std::numeric_limits<Real>::infinity();
  }
  return std::sqrt(norm_squared);
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::validateFiniteLocalPoint(const LocalPoint & point,
                                                                        const char * stage) const
{
  const auto check = [&](const char * field, const auto & value)
  { validateFiniteValue(value, "local constitutive state", stage, field); };

  check("p", point.p);
  check("q", point.q);
  check("f", point.f);

  for (const auto & residual : point.residual)
    check("local residual", residual);

  check("effective inelastic strain increment", point.effective_inelastic_strain_increment);

  validateFiniteTensor(point.inelastic_strain_increment,
                       "local constitutive state",
                       stage,
                       "inelastic strain increment");

  for (const auto & row : point.jacobian)
    for (const auto & value : row)
      check("local Jacobian", value);

  check("effective hydrostatic stress", point.hydrostatic_stress.effective_hydro_stress);
  check("effective-hydrostatic porosity derivative", point.hydrostatic_stress.deffective_hydro_df);
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::ScaledLocalJacobian
PorousViscoplasticityStressUpdateTempl<is_ad>::scaledJacobian(
    const LocalJacobian & jacobian, const LocalSolveContext & context) const
{
  auto scaled = ScaledLocalJacobian{};
  const std::array<Real, LOCAL_SYSTEM_SIZE> variable_scales = {
      context.p_scale, context.q_scale, context.porosity_variable_scale};
  const std::array<Real, LOCAL_SYSTEM_SIZE> residual_scales = {
      context.p_scale, context.q_scale, context.porosity_merit_scale};

  for (auto row = 0u; row < LOCAL_SYSTEM_SIZE; ++row)
    for (auto column = 0u; column < LOCAL_SYSTEM_SIZE; ++column)
      scaled[row][column] = MetaPhysicL::raw_value(jacobian[row][column]) *
                            variable_scales[column] / residual_scales[row];

  return scaled;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint
PorousViscoplasticityStressUpdateTempl<is_ad>::reconstructImplicitSensitivity(
    const LocalPoint & point, const LocalSolveContext & context)
{
  if constexpr (!is_ad)
    return point;
  else
  {
    /*
     * Project the AD derivatives of the accepted local coordinates onto the implicit constitutive
     * system R(x,z)=0:
     *
     *   dx/dz = -(dR/dx)^-1 dR/dz.
     *
     * The local Newton solve uses a raw-value Real Jacobian. Carrying AD residuals through those
     * corrections is only asymptotically exact because the differentiated iteration omits terms
     * proportional to the remaining local residual. Line searches, active-set transitions, and
     * reduced-porosity globalization may also alter or discard derivative history. Solve one
     * derivative-only correction at fixed raw p, q, and f so every AD branch returns a consistent
     * algorithmic sensitivity.
     */
    validateFiniteLocalPoint(point, "implicit AD sensitivity reconstruction");

    const auto jacobian = scaledJacobian(point.jacobian, context);
    const auto residual = scaledResidual(point.residual, context);
    const auto rhs = LocalResidual{-residual[P_INDEX], -residual[Q_INDEX], -residual[F_INDEX]};
    auto correction = LocalResidual{};
    if (!solveLinearSystem(jacobian, rhs, correction))
      mooseException("In ",
                     this->_name,
                     ": singular analytical coupled local (p,q,f) Jacobian during implicit AD "
                     "sensitivity reconstruction.");

    auto coordinates = point.coordinates();
    const auto dp = context.p_scale * correction[P_INDEX];
    const auto dq = context.q_scale * correction[Q_INDEX];
    const auto df = context.porosity_variable_scale * correction[F_INDEX];

    coordinates.p += dp - MetaPhysicL::raw_value(dp);
    coordinates.q += dq - MetaPhysicL::raw_value(dq);
    coordinates.f += df - MetaPhysicL::raw_value(df);

    auto reconstructed = evaluateLocalPoint(coordinates, context, point.porosity_branch);
    validateFiniteLocalPoint(reconstructed, "implicit AD sensitivity reconstruction");
    return reconstructed;
  }
}

template <bool is_ad>
RankFourTensor
PorousViscoplasticityStressUpdateTempl<is_ad>::computeConsistentTangent(
    const LocalPoint & point, const LocalSolveContext & context) const
{
  /*
   * The converged local coordinates x=(p,q,f) satisfy R(x,p_trial,q_trial)=0. For an
   * isotropic elasticity tensor the scalar equations depend on the trial stress only through
   * p_trial and q_trial, so implicit differentiation gives
   *
   *   J dx/dp_trial = (1,0,0)^T,
   *   J dx/dq_trial = (0,1,0)^T.
   *
   * Solve the already-scaled 3x3 system to retain the conditioning used by the local Newton
   * solve, then reconstruct d sigma/d epsilon from sigma = p I + q n, where
   * n = s_trial/q_trial. No finite-difference perturbation is used.
   */
  validateFiniteLocalPoint(point, "consistent tangent evaluation");

  const auto jacobian = scaledJacobian(point.jacobian, context);
  const std::array<Real, LOCAL_SYSTEM_SIZE> variable_scales = {
      context.p_scale, context.q_scale, context.porosity_variable_scale};
  const std::array<Real, LOCAL_SYSTEM_SIZE> residual_scales = {
      context.p_scale, context.q_scale, context.porosity_merit_scale};
  const auto solve_trial_sensitivity = [&](const LocalVariableIndex trial_component)
  {
    auto rhs = FixedVector<Real, LOCAL_SYSTEM_SIZE>{0.0, 0.0, 0.0};
    rhs[trial_component] = 1.0 / residual_scales[trial_component];

    auto scaled_sensitivity = FixedVector<Real, LOCAL_SYSTEM_SIZE>{};
    if (!solveLinearSystem(jacobian, rhs, scaled_sensitivity))
      mooseException("In ",
                     this->_name,
                     ": singular analytical coupled local (p,q,f) Jacobian during consistent "
                     "tangent evaluation.");

    return FixedVector<Real, LOCAL_SYSTEM_SIZE>{
        variable_scales[P_INDEX] * scaled_sensitivity[P_INDEX],
        variable_scales[Q_INDEX] * scaled_sensitivity[Q_INDEX],
        variable_scales[F_INDEX] * scaled_sensitivity[F_INDEX]};
  };

  const auto sensitivity_p_trial = solve_trial_sensitivity(P_INDEX);
  const auto q_trial = MetaPhysicL::raw_value(context.trial_equiv_stress);
  const auto has_deviatoric_direction = q_trial > this->_minimum_stress_magnitude;
  const auto sensitivity_q_trial = has_deviatoric_direction
                                       ? solve_trial_sensitivity(Q_INDEX)
                                       : FixedVector<Real, LOCAL_SYSTEM_SIZE>{0.0, 0.0, 0.0};

  const auto elasticity =
      [&](const unsigned int i, const unsigned int j, const unsigned int k, const unsigned int l)
  { return MetaPhysicL::raw_value(context.elasticity_tensor(i, j, k, l)); };

  const auto identity = RankTwoTensor(RankTwoTensor::initIdentity);
  auto hydro_trial_gradient = RankTwoTensor();
  hydro_trial_gradient.zero();
  auto equiv_trial_gradient = RankTwoTensor();
  equiv_trial_gradient.zero();
  auto dev_direction = RankTwoTensor();
  dev_direction.zero();

  if (has_deviatoric_direction)
    for (auto i = 0u; i < 3; ++i)
      for (auto j = 0u; j < 3; ++j)
        dev_direction(i, j) = MetaPhysicL::raw_value(context.trial_dev_stress(i, j)) / q_trial;

  for (auto k = 0u; k < 3; ++k)
    for (auto l = 0u; l < 3; ++l)
    {
      for (auto i = 0u; i < 3; ++i)
        hydro_trial_gradient(k, l) += elasticity(i, i, k, l) / 3.0;

      if (has_deviatoric_direction)
        for (auto i = 0u; i < 3; ++i)
          for (auto j = 0u; j < 3; ++j)
            equiv_trial_gradient(k, l) += 1.5 * dev_direction(i, j) * elasticity(i, j, k, l);
    }

  auto tangent = RankFourTensor();
  tangent.zero();

  if (!has_deviatoric_direction)
  {
    /*
     * The local q equation is q=0 on the small-deviatoric-stress branch, while the trial
     * deviatoric stress remains elastic. Only the hydrostatic response is modified.
     */
    const auto dp_dp_trial = sensitivity_p_trial[P_INDEX];
    for (auto i = 0u; i < 3; ++i)
      for (auto j = 0u; j < 3; ++j)
        for (auto k = 0u; k < 3; ++k)
          for (auto l = 0u; l < 3; ++l)
            tangent(i, j, k, l) = elasticity(i, j, k, l) +
                                  identity(i, j) * (dp_dp_trial - 1.0) * hydro_trial_gradient(k, l);
  }
  else
  {
    const auto dp_dp_trial = sensitivity_p_trial[P_INDEX];
    const auto dq_dp_trial = sensitivity_p_trial[Q_INDEX];
    const auto dp_dq_trial = sensitivity_q_trial[P_INDEX];
    const auto dq_dq_trial = sensitivity_q_trial[Q_INDEX];
    const auto q = MetaPhysicL::raw_value(point.q);

    auto p_gradient = RankTwoTensor();
    p_gradient.zero();
    auto q_gradient = RankTwoTensor();
    q_gradient.zero();
    for (auto k = 0u; k < 3; ++k)
      for (auto l = 0u; l < 3; ++l)
      {
        p_gradient(k, l) =
            dp_dp_trial * hydro_trial_gradient(k, l) + dp_dq_trial * equiv_trial_gradient(k, l);
        q_gradient(k, l) =
            dq_dp_trial * hydro_trial_gradient(k, l) + dq_dq_trial * equiv_trial_gradient(k, l);
      }

    for (auto i = 0u; i < 3; ++i)
      for (auto j = 0u; j < 3; ++j)
        for (auto k = 0u; k < 3; ++k)
          for (auto l = 0u; l < 3; ++l)
          {
            const auto dev_trial_gradient =
                elasticity(i, j, k, l) - identity(i, j) * hydro_trial_gradient(k, l);
            const auto dev_direction_gradient =
                (dev_trial_gradient - dev_direction(i, j) * equiv_trial_gradient(k, l)) / q_trial;

            tangent(i, j, k, l) = identity(i, j) * p_gradient(k, l) +
                                  dev_direction(i, j) * q_gradient(k, l) +
                                  q * dev_direction_gradient;
          }
  }

  for (auto i = 0u; i < 3; ++i)
    for (auto j = 0u; j < 3; ++j)
      for (auto k = 0u; k < 3; ++k)
        for (auto l = 0u; l < 3; ++l)
          if (!std::isfinite(tangent(i, j, k, l)))
            mooseException("In ",
                           this->_name,
                           ": nonfinite consistent tangent component (",
                           i,
                           ",",
                           j,
                           ",",
                           k,
                           ",",
                           l,
                           ") = ",
                           tangent(i, j, k, l),
                           ".");

  return tangent;
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint>
PorousViscoplasticityStressUpdateTempl<is_ad>::backtrackingLineSearch(
    const LocalPoint & point,
    const LocalResidual & correction_scaled,
    const Real initial_alpha,
    const LocalResidualScope residual_scope,
    const LocalSolveContext & context)
{
  const auto coupled_scope = residual_scope == LocalResidualScope::COUPLED;
  auto alpha = initial_alpha;
  const auto residual_norm = residualNorm(scaledResidual(point.residual, context), residual_scope);

  for (auto backtrack = 0u; backtrack <= _local_newton_max_backtracks; ++backtrack)
  {
    const auto trial_coordinates =
        LocalCoordinates{point.p + alpha * context.p_scale * correction_scaled[P_INDEX],
                         point.q + alpha * context.q_scale * correction_scaled[Q_INDEX],
                         coupled_scope ? point.f + alpha * context.porosity_variable_scale *
                                                       correction_scaled[F_INDEX]
                                       : point.f};

    const auto p_raw = MetaPhysicL::raw_value(trial_coordinates.p);
    const auto q_raw = MetaPhysicL::raw_value(trial_coordinates.q);
    const auto f_raw = MetaPhysicL::raw_value(trial_coordinates.f);

    const auto mechanical_admissible =
        std::isfinite(p_raw) && std::isfinite(q_raw) && trial_coordinates.q >= 0.0;
    const auto porosity_admissible =
        !coupled_scope || (std::isfinite(f_raw) && trial_coordinates.f >= _minimum_porosity &&
                           trial_coordinates.f < 1.0);

    if (mechanical_admissible && porosity_admissible)
    {
      try
      {
        auto trial = evaluateLocalPoint(trial_coordinates, context, point.porosity_branch);

        if (residualNorm(scaledResidual(trial.residual, context), residual_scope) < residual_norm)
          return trial;
      }
      catch (const MooseException &)
      {
      }
    }

    alpha *= 0.5;
  }

  return std::nullopt;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::boundedBeginningPorosity() const
{
  auto porosity_begin = this->_intermediate_porosity;
  const auto raw_porosity_begin = MetaPhysicL::raw_value(porosity_begin);
  if (!std::isfinite(raw_porosity_begin))
    mooseException("In ",
                   this->_name,
                   ": nonfinite beginning-of-substep porosity at qp ",
                   this->_qp,
                   ". porosity = ",
                   raw_porosity_begin,
                   ".");

  if (porosity_begin < _minimum_porosity)
  {
    if (_minimum_porosity - porosity_begin > _porosity_bound_tolerance)
      mooseException("In ",
                     this->_name,
                     ": beginning-of-substep porosity ",
                     MetaPhysicL::raw_value(porosity_begin),
                     " is below minimum_porosity ",
                     _minimum_porosity,
                     ".");

    porosity_begin = _minimum_porosity;
  }

  if (porosity_begin >= 1.0)
    mooseException("In ",
                   this->_name,
                   ": inadmissible beginning-of-substep porosity ",
                   MetaPhysicL::raw_value(porosity_begin),
                   ".");

  return porosity_begin;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::initializeLocalSolveScales(
    LocalSolveContext & context) const
{
  using std::abs;
  using std::max;
  using std::min;

  /*
   * Do not use |p_eff| as the scale for the matrix equilibrium equations. At small porosity the
   * additional hydrostatic driving pressure can greatly exceed p and q; using p_eff then makes
   * a mechanically poor state appear converged and prematurely triggers the reduced-f rescue.
   *
   * Instead use matrix-elastic stress scales associated with max_inelastic_increment. For an
   * isotropic tensor, K=(C1111+2 C1122)/3 and G=C1212.
   */
  const auto bulk_modulus = abs(MetaPhysicL::raw_value(
      (context.elasticity_tensor(0, 0, 0, 0) + 2.0 * context.elasticity_tensor(0, 0, 1, 1)) / 3.0));
  const auto shear_modulus = abs(MetaPhysicL::raw_value(context.elasticity_tensor(0, 1, 0, 1)));

  context.p_scale = max({abs(MetaPhysicL::raw_value(context.p_trial)),
                         bulk_modulus * this->_max_inelastic_increment,
                         this->_minimum_stress_magnitude,
                         1.0});
  context.q_scale = max({abs(MetaPhysicL::raw_value(context.trial_equiv_stress)),
                         3.0 * shear_modulus * this->_max_inelastic_increment,
                         this->_minimum_stress_magnitude,
                         1.0});

  /*
   * Keep separate porosity scales for the Newton variable, line-search merit, and physical
   * convergence check. The variable scale may reflect a large admissible porosity change for good
   * conditioning, but using that same scale for the residual can make the line search effectively
   * blind to Rf when the accepted porosity is very small. Keep the merit scale within one decade of
   * the physical convergence scale while retaining the larger variable scale.
   */
  context.porosity_convergence_scale = max({abs(MetaPhysicL::raw_value(context.porosity_begin)),
                                            _local_porosity_scale_floor,
                                            _minimum_porosity,
                                            10.0 * _porosity_bound_tolerance});
  const auto characteristic_increment =
      max(0.0,
          (1.0 - MetaPhysicL::raw_value(this->_porosity_old[this->_qp])) *
              this->_max_inelastic_increment);
  context.porosity_variable_scale =
      max(context.porosity_convergence_scale, characteristic_increment);
  context.porosity_merit_scale =
      max(context.porosity_convergence_scale,
          min(characteristic_increment, 10.0 * context.porosity_convergence_scale));

  context.reduced_probe_tolerance = max(_local_newton_tolerance, 1.0e-5);
  context.reduced_final_tolerance =
      min(_local_newton_tolerance, 0.1 * _local_newton_stagnation_tolerance);
}

template <bool is_ad>
bool
PorousViscoplasticityStressUpdateTempl<is_ad>::denseLimitActive(
    const LocalSolveContext & context) const
{
  /*
   * At the zero-porosity lower bound the spherical LPS surface reduces exactly to dense J2
   * Norton creep. Do not carry that limit through the porous p-q-f active set: the porosity and
   * hydrostatic LPS derivatives are then physically inactive and only add numerical conditioning
   * to an otherwise one-dimensional deviatoric return.
   *
   * Use the same lower-bound tolerance as the active-set initialization. This also prevents tiny
   * roundoff-level positive porosities generated by an exactly isochoric dense update from
   * needlessly switching back into the porous solve on the next step.
   */
  return _minimum_porosity == 0.0 &&
         MetaPhysicL::raw_value(context.porosity_begin) <= _porosity_bound_tolerance;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint
PorousViscoplasticityStressUpdateTempl<is_ad>::evaluateDenseLimitPoint(
    const GenericReal<is_ad> & q, const LocalSolveContext & context) const
{
  using std::pow;

  auto point = LocalPoint{};
  point.porosity_branch = PorosityBranch::FLOOR;
  point.f = 0.0;
  point.inelastic_strain_increment.zero();

  const auto has_deviatoric_direction =
      context.trial_equiv_stress > this->_minimum_stress_magnitude;

  point.q = has_deviatoric_direction ? q : GenericReal<is_ad>(0.0);

  auto dev_direction = GenericRankTwoTensor<is_ad>();
  dev_direction.zero();
  if (has_deviatoric_direction)
    dev_direction = context.trial_dev_stress / context.trial_equiv_stress;

  auto creep_rate = GenericReal<is_ad>(0.0);
  auto dcreep_rate_dq = GenericReal<is_ad>(0.0);
  if (has_deviatoric_direction)
    for (auto law_index = std::size_t{0}; law_index < _creep_laws.size(); ++law_index)
    {
      const auto coefficient = creepCoefficient(law_index);
      if (MetaPhysicL::raw_value(coefficient) == 0.0)
        continue;

      const auto & law = _creep_laws[law_index];
      creep_rate += computeCreepRate(law, coefficient, point.q);
      dcreep_rate_dq += law.power * coefficient * pow(point.q, law.power - 1.0);
    }

  point.effective_inelastic_strain_increment = creep_rate * this->constitutiveTimeStep();
  point.inelastic_strain_increment =
      dev_direction * (1.5 * point.effective_inelastic_strain_increment);

  const auto stress_calculated =
      context.elasticity_tensor *
      (context.elastic_strain_old + context.trial_elastic_strain_increment -
       point.inelastic_strain_increment);
  const auto p_calculated = stress_calculated.trace() / 3.0;
  const auto dev_stress_calculated = stress_calculated.deviatoric();
  const auto q_calculated = has_deviatoric_direction
                                ? 1.5 * dev_direction.doubleContraction(dev_stress_calculated)
                                : GenericReal<is_ad>(0.0);

  point.p = p_calculated;
  point.hydrostatic_stress = evaluateHydrostaticStress(point.p, point.f);
  point.residual[P_INDEX] = 0.0;
  point.residual[Q_INDEX] = has_deviatoric_direction ? point.q - q_calculated : point.q;
  point.residual[F_INDEX] = point.f;

  point.jacobian[P_INDEX][P_INDEX] = 1.0;
  point.jacobian[F_INDEX][F_INDEX] = 1.0;

  if (has_deviatoric_direction)
  {
    const auto dinelastic_dq =
        dev_direction * (1.5 * this->constitutiveTimeStep() * dcreep_rate_dq);
    const auto elastic_response = context.elasticity_tensor * dinelastic_dq;

    point.jacobian[P_INDEX][Q_INDEX] = elastic_response.trace() / 3.0;
    point.jacobian[Q_INDEX][Q_INDEX] =
        1.0 + 1.5 * dev_direction.doubleContraction(elastic_response.deviatoric());
  }
  else
    point.jacobian[Q_INDEX][Q_INDEX] = 1.0;

  return point;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint
PorousViscoplasticityStressUpdateTempl<is_ad>::solveDenseLimit(
    const LocalSolveContext & context) const
{
  using std::abs;
  using std::max;
  using std::min;

  if (context.trial_equiv_stress <= this->_minimum_stress_magnitude)
    return evaluateDenseLimitPoint(GenericReal<is_ad>(0.0), context);

  auto q = context.trial_equiv_stress;
  auto point = evaluateDenseLimitPoint(q, context);
  const auto q_scale = max({abs(MetaPhysicL::raw_value(context.trial_equiv_stress)),
                            this->_minimum_stress_magnitude,
                            1.0});

  // The dense scalar solve is inexpensive, so solve it at least this tightly even when the
  // general porous local tolerance is looser.
  const auto tolerance = min(_local_newton_tolerance, 1.0e-12);
  auto converged = false;

  for (auto iteration = 0u; iteration < _local_newton_max_iterations; ++iteration)
  {
    validateFiniteLocalPoint(point, "dense-limit Newton iteration");
    const auto residual = MetaPhysicL::raw_value(point.residual[Q_INDEX]);
    if (abs(residual) / q_scale <= tolerance)
    {
      converged = true;
      break;
    }

    const auto jacobian = MetaPhysicL::raw_value(point.jacobian[Q_INDEX][Q_INDEX]);
    if (!std::isfinite(jacobian) || jacobian <= 0.0)
      mooseException(
          "In ", this->_name, ": invalid dense-limit J2 local Jacobian dRq/dq = ", jacobian, ".");

    const auto correction = -point.residual[Q_INDEX] / jacobian;
    auto alpha = Real(1.0);
    auto q_candidate = q + correction;

    while (MetaPhysicL::raw_value(q_candidate) < 0.0 &&
           alpha > std::numeric_limits<Real>::epsilon())
    {
      alpha *= 0.5;
      q_candidate = q + alpha * correction;
    }

    if (MetaPhysicL::raw_value(q_candidate) < 0.0)
      mooseException("In ", this->_name, ": dense-limit J2 Newton could not retain q >= 0.");

    q = q_candidate;
    point = evaluateDenseLimitPoint(q, context);
  }

  if (!converged)
  {
    const auto residual = abs(MetaPhysicL::raw_value(point.residual[Q_INDEX])) / q_scale;
    if (residual > tolerance)
      mooseException("In ",
                     this->_name,
                     ": dense-limit J2 local Newton failed to converge in ",
                     _local_newton_max_iterations,
                     " iterations. Final scaled residual = ",
                     residual,
                     ".");
  }

  if constexpr (is_ad)
  {
    /*
     * The Newton value is already converged. Project only its AD sensitivity onto the exact
     * implicit scalar solution Rq(q,z)=0. Dividing by the raw local Jacobian is intentional:
     *
     *   dq/dz <- dq/dz - (dRq/dz) / (dRq/dq),
     *
     * while subtracting the raw correction leaves the converged q value unchanged.
     */
    const auto jacobian = MetaPhysicL::raw_value(point.jacobian[Q_INDEX][Q_INDEX]);
    if (!std::isfinite(jacobian) || jacobian <= 0.0)
      mooseException("In ",
                     this->_name,
                     ": invalid dense-limit J2 local Jacobian during AD sensitivity projection: ",
                     jacobian,
                     ".");

    const auto correction = -point.residual[Q_INDEX] / jacobian;
    q += correction - MetaPhysicL::raw_value(correction);
    point = evaluateDenseLimitPoint(q, context);
  }

  validateFiniteLocalPoint(point, "dense-limit final consistency check");
  return point;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint
PorousViscoplasticityStressUpdateTempl<is_ad>::solvePorosityActiveSet(
    const LocalSolveContext & context, bool & reduced_porosity_attempted)
{
  const auto initial_branch =
      context.porosity_begin <= _minimum_porosity + _porosity_bound_tolerance
          ? PorosityBranch::FLOOR
          : PorosityBranch::FREE;
  const auto initial_f = initial_branch == PorosityBranch::FLOOR
                             ? GenericReal<is_ad>(_minimum_porosity)
                             : context.porosity_begin;

  const auto initial_coordinates =
      LocalCoordinates{context.p_trial, context.trial_equiv_stress, initial_f};
  auto point = evaluateLocalPoint(initial_coordinates, context, initial_branch);

  auto evaluate_branch_at_floor = [&](const PorosityBranch branch)
  {
    const auto coordinates = LocalCoordinates{point.p, point.q, _minimum_porosity};
    point = evaluateLocalPoint(coordinates, context, branch);
  };

  for (auto active_set_iteration = 0u; active_set_iteration < 4; ++active_set_iteration)
  {
    const auto solve = solveCoupledNewton(point, context, reduced_porosity_attempted);
    point = solve.point;

    if (solve.activate_floor)
    {
      if (this->_verbose)
        Moose::out << "Porosity active set: activating floor at "
                   << "active_set_iteration = " << active_set_iteration
                   << ", p = " << MetaPhysicL::raw_value(point.p)
                   << ", q = " << MetaPhysicL::raw_value(point.q)
                   << ", f = " << MetaPhysicL::raw_value(point.f)
                   << ", implied f = " << MetaPhysicL::raw_value(impliedPorosity(point))
                   << std::endl;

      evaluate_branch_at_floor(PorosityBranch::FLOOR);
      continue;
    }

    const auto branch = point.porosity_branch;

    /* A free solution away from the lower bound already satisfies the active-set condition. */
    const auto at_floor = point.f <= _minimum_porosity + _porosity_bound_tolerance;
    if (branch == PorosityBranch::FREE && !at_floor)
      return point;

    /*
     * Evaluate the unconstrained porosity residual at the floor for complementarity:
     *
     *   active branch: Rf_free >= -tol -> retain floor
     *   free branch:   Rf_free <=  tol -> retain free branch.
     */
    const auto free_coordinates = LocalCoordinates{point.p, point.q, _minimum_porosity};
    const auto free_residual_at_floor =
        evaluateLocalPoint(free_coordinates, context, PorosityBranch::FREE).residual[F_INDEX];
    const auto floor_required = branch == PorosityBranch::FLOOR
                                    ? free_residual_at_floor >= -_porosity_bound_tolerance
                                    : free_residual_at_floor > _porosity_bound_tolerance;
    const auto next_branch = floor_required ? PorosityBranch::FLOOR : PorosityBranch::FREE;

    if (next_branch == branch)
      return point;

    if (this->_verbose)
    {
      if (next_branch == PorosityBranch::FLOOR)
        Moose::out << "Porosity active set: free solution requires floor."
                   << " active_set_iteration = " << active_set_iteration
                   << ", Rf_free_at_floor = " << MetaPhysicL::raw_value(free_residual_at_floor)
                   << std::endl;
      else
        Moose::out << "Porosity active set: releasing floor at "
                   << "active_set_iteration = " << active_set_iteration
                   << ", p = " << MetaPhysicL::raw_value(point.p)
                   << ", q = " << MetaPhysicL::raw_value(point.q)
                   << ", Rf_free_at_floor = " << MetaPhysicL::raw_value(free_residual_at_floor)
                   << std::endl;
    }

    evaluate_branch_at_floor(next_branch);
  }

  mooseException("In ",
                 this->_name,
                 ": porosity lower-bound active set failed to stabilize."
                 "\n  p = ",
                 MetaPhysicL::raw_value(point.p),
                 "\n  q = ",
                 MetaPhysicL::raw_value(point.q),
                 "\n  f = ",
                 MetaPhysicL::raw_value(point.f),
                 "\n  f_begin = ",
                 MetaPhysicL::raw_value(context.porosity_begin),
                 "\n  f_min = ",
                 _minimum_porosity,
                 "\n  floor active = ",
                 point.porosityFloorActive(),
                 ".");

  return point;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::commitLocalPoint(
    const LocalPoint & point,
    const LocalSolveContext & context,
    GenericRankTwoTensor<is_ad> & elastic_strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    GenericRankTwoTensor<is_ad> & stress,
    GenericReal<is_ad> & effective_inelastic_strain_increment)
{
  validateFiniteLocalPoint(point, "local state commit");

  const auto candidate_inelastic_strain_increment = point.inelastic_strain_increment;
  const auto candidate_elastic_strain_increment =
      context.trial_elastic_strain_increment - candidate_inelastic_strain_increment;
  const auto candidate_stress =
      context.elasticity_tensor * (context.elastic_strain_old + candidate_elastic_strain_increment);
  const auto candidate_effective_inelastic_strain_increment =
      point.effective_inelastic_strain_increment;

  validateFiniteTensor(candidate_inelastic_strain_increment,
                       "committed constitutive state",
                       "local state commit",
                       "inelastic strain increment");
  validateFiniteTensor(candidate_elastic_strain_increment,
                       "committed constitutive state",
                       "local state commit",
                       "elastic strain increment");
  validateFiniteTensor(
      candidate_stress, "committed constitutive state", "local state commit", "stress");
  validateFiniteValue(candidate_effective_inelastic_strain_increment,
                      "committed constitutive state",
                      "local state commit",
                      "effective inelastic strain increment");

  inelastic_strain_increment = candidate_inelastic_strain_increment;
  elastic_strain_increment = candidate_elastic_strain_increment;
  stress = candidate_stress;
  effective_inelastic_strain_increment = candidate_effective_inelastic_strain_increment;

  const auto q_flow = context.trial_equiv_stress > this->_minimum_stress_magnitude
                          ? point.q
                          : GenericReal<is_ad>(0.0);
  this->setGaugeStresses(q_flow, point.hydrostatic_stress.effective_hydro_stress, point.f);

  this->_hydro_stress = point.p;
  const auto raw_porosity = MetaPhysicL::raw_value(point.f);

  if (raw_porosity < _minimum_porosity)
  {
    if (_minimum_porosity - raw_porosity > _porosity_bound_tolerance)
      mooseException("In ",
                     this->_name,
                     ": converged porosity ",
                     raw_porosity,
                     " is below minimum_porosity ",
                     _minimum_porosity,
                     ".");

    this->_intermediate_porosity = GenericReal<is_ad>(_minimum_porosity);
  }
  else
    this->_intermediate_porosity = point.f;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint
PorousViscoplasticityStressUpdateTempl<is_ad>::verifyConvergedPoint(
    const LocalPoint & point, const LocalSolveContext & context)
{
  auto verified = evaluateLocalPoint(point.coordinates(), context, point.porosity_branch);
  validateFiniteLocalPoint(verified, "final coupled consistency check");

  const auto residual_norm = convergenceResidualNorm(verified.residual, context);
  if (residual_norm > _local_newton_stagnation_tolerance)
    mooseException("In ",
                   this->_name,
                   ": final analytical coupled local consistency check failed with scaled "
                   "residual norm = ",
                   residual_norm,
                   ".");

  return verified;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint
PorousViscoplasticityStressUpdateTempl<is_ad>::verifyReducedConvergedPoint(
    const LocalSolveResult & reduced, const LocalSolveContext & context)
{
  using std::abs;

  auto verified = evaluateLocalPoint(reduced.point.coordinates(), context, PorosityBranch::FREE);
  validateFiniteLocalPoint(verified, "final reduced-porosity consistency check");
  const auto mechanical_norm = physicalMechanicalResidualNorm(verified.residual,
                                                              context.p_trial,
                                                              context.trial_equiv_stress,
                                                              this->_minimum_stress_magnitude);
  const auto porosity_residual_magnitude =
      abs(MetaPhysicL::raw_value(verified.residual[F_INDEX])) / context.porosity_convergence_scale;

  if (mechanical_norm > context.reduced_final_tolerance)
    mooseException("In ",
                   this->_name,
                   ": final reduced-porosity mechanical consistency check failed with physically "
                   "scaled "
                   "Rp-Rq norm = ",
                   mechanical_norm,
                   ".");

  if (porosity_residual_magnitude > _local_newton_stagnation_tolerance &&
      reduced.porosity_error_bound > _porosity_bound_tolerance)
    mooseException("In ",
                   this->_name,
                   ": final reduced-porosity consistency check failed. Scaled |Rf| = ",
                   porosity_residual_magnitude,
                   ", porosity error bound = ",
                   reduced.porosity_error_bound,
                   ".");

  return verified;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::impliedPorosity(const LocalPoint & point) const
{
  return point.f - point.residual[F_INDEX];
}

template <bool is_ad>
bool
PorousViscoplasticityStressUpdateTempl<is_ad>::freeIncrementReachesPorosityFloor(
    const LocalPoint & point) const
{
  return !point.porosityFloorActive() &&
         impliedPorosity(point) <= _minimum_porosity + _porosity_bound_tolerance;
}

template <bool is_ad>
Real
PorousViscoplasticityStressUpdateTempl<is_ad>::reducedPorosityResidual(
    const LocalPoint & point) const
{
  return MetaPhysicL::raw_value(point.residual[F_INDEX]);
}

template <bool is_ad>
bool
PorousViscoplasticityStressUpdateTempl<is_ad>::reducedPointConverged(
    const LocalPoint & point, const LocalSolveContext & context) const
{
  using std::abs;

  const auto mechanical_norm = physicalMechanicalResidualNorm(
      point.residual, context.p_trial, context.trial_equiv_stress, this->_minimum_stress_magnitude);
  const auto scaled_porosity_residual =
      MetaPhysicL::raw_value(point.residual[F_INDEX]) / context.porosity_convergence_scale;
  if (!std::isfinite(scaled_porosity_residual))
    return false;

  const auto porosity_residual_magnitude = abs(scaled_porosity_residual);

  return mechanical_norm <= context.reduced_final_tolerance &&
         porosity_residual_magnitude <= _local_newton_stagnation_tolerance;
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint>
PorousViscoplasticityStressUpdateTempl<is_ad>::solveReducedPoint(const LocalCoordinates & seed,
                                                                 const LocalSolveContext & context,
                                                                 Real & best_abs_rf_scaled)
{
  using std::abs;

  auto point = solveMechanicalAtFixedPorosity(seed, context.reduced_probe_tolerance, context);
  if (!point)
    return std::nullopt;

  const auto initial_reduced_residual = reducedPorosityResidual(*point);
  if (!std::isfinite(initial_reduced_residual))
    return std::nullopt;

  auto scaled_residual_magnitude =
      abs(initial_reduced_residual) / context.porosity_convergence_scale;

  /*
   * A near-root probe must be projected onto the tightly equilibrated mechanical manifold before
   * its reduced residual is used for convergence and safeguarded bracket decisions.
   */
  if (scaled_residual_magnitude <= context.reduced_probe_tolerance)
  {
    auto tight = solveMechanicalAtFixedPorosity(
        point->coordinates(), context.reduced_final_tolerance, context);
    if (!tight)
      return std::nullopt;

    point = tight;
    const auto tightened_reduced_residual = reducedPorosityResidual(*point);
    if (!std::isfinite(tightened_reduced_residual))
      return std::nullopt;
    scaled_residual_magnitude =
        abs(tightened_reduced_residual) / context.porosity_convergence_scale;
  }

  best_abs_rf_scaled = std::min(best_abs_rf_scaled, scaled_residual_magnitude);

  return point;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalCoordinates
PorousViscoplasticityStressUpdateTempl<is_ad>::reducedBracketMidpoint(
    const LocalPoint & lower, const LocalPoint & upper) const
{
  const auto f_lower = MetaPhysicL::raw_value(lower.f);
  const auto f_upper = MetaPhysicL::raw_value(upper.f);
  const auto bracket_width = f_upper - f_lower;

  auto fraction = 0.5;
  auto f_mid = 0.5 * (lower.f + upper.f);

  if (f_lower > 0.0 && f_upper / f_lower > 4.0)
  {
    const auto f_mid_raw = std::sqrt(f_lower * f_upper);
    f_mid = f_mid_raw;
    fraction = (f_mid_raw - f_lower) / bracket_width;
  }

  return LocalCoordinates{
      lower.p + fraction * (upper.p - lower.p), lower.q + fraction * (upper.q - lower.q), f_mid};
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint>
PorousViscoplasticityStressUpdateTempl<is_ad>::solveMechanicalAtFixedPorosity(
    const LocalCoordinates & seed, const Real tolerance, const LocalSolveContext & context)
{

  std::optional<LocalPoint> point;
  try
  {
    point = evaluateLocalPoint(seed, context, PorosityBranch::FREE);
  }
  catch (const MooseException &)
  {
    return std::nullopt;
  }

  constexpr auto max_iterations = 15u;

  for (auto iteration = 0u; iteration < max_iterations; ++iteration)
  {
    const auto residual = scaledResidual(point->residual, context);
    if (physicalMechanicalResidualNorm(point->residual,
                                       context.p_trial,
                                       context.trial_equiv_stress,
                                       this->_minimum_stress_magnitude) <= tolerance)
      return point;

    // Scaled 2x2 mechanical block. p and q intentionally use different matrix-stress scales.
    const auto local_jacobian = scaledJacobian(point->jacobian, context);
    const auto mechanical_jacobian = FixedMatrix<Real, 2>{
        {{local_jacobian[P_INDEX][P_INDEX], local_jacobian[P_INDEX][Q_INDEX]},
         {local_jacobian[Q_INDEX][P_INDEX], local_jacobian[Q_INDEX][Q_INDEX]}}};

    const auto mechanical_rhs =
        FixedVector<GenericReal<is_ad>, 2>{-residual[P_INDEX], -residual[Q_INDEX]};
    auto mechanical_correction = FixedVector<GenericReal<is_ad>, 2>{};

    if (!solveLinearSystem(mechanical_jacobian, mechanical_rhs, mechanical_correction))
      return std::nullopt;

    const auto correction_scaled =
        LocalResidual{mechanical_correction[0], mechanical_correction[1], GenericReal<is_ad>(0.0)};

    auto candidate = backtrackingLineSearch(*point,
                                            correction_scaled,
                                            _local_newton_relaxation,
                                            LocalResidualScope::MECHANICAL,
                                            context);
    if (!candidate)
      return std::nullopt;

    point = *candidate;
  }

  return physicalMechanicalResidualNorm(point->residual,
                                        context.p_trial,
                                        context.trial_equiv_stress,
                                        this->_minimum_stress_magnitude) <= tolerance
             ? point
             : std::nullopt;
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::ReducedPorosityTangent>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeReducedPorosityTangent(
    const LocalJacobian & jacobian) const
{
  const auto mechanical_jacobian = FixedMatrix<GenericReal<is_ad>, 2>{
      {{jacobian[P_INDEX][P_INDEX], jacobian[P_INDEX][Q_INDEX]},
       {jacobian[Q_INDEX][P_INDEX], jacobian[Q_INDEX][Q_INDEX]}}};
  const auto porosity_coupling =
      FixedVector<GenericReal<is_ad>, 2>{jacobian[P_INDEX][F_INDEX], jacobian[Q_INDEX][F_INDEX]};
  auto mechanical_response = FixedVector<GenericReal<is_ad>, 2>{};

  if (!solveLinearSystem(mechanical_jacobian, porosity_coupling, mechanical_response))
    return std::nullopt;

  auto tangent = ReducedPorosityTangent{};
  tangent.dp_df = -mechanical_response[0];
  tangent.dq_df = -mechanical_response[1];
  tangent.drhat_df = jacobian[F_INDEX][F_INDEX] -
                     jacobian[F_INDEX][P_INDEX] * mechanical_response[0] -
                     jacobian[F_INDEX][Q_INDEX] * mechanical_response[1];

  if (!std::isfinite(MetaPhysicL::raw_value(tangent.drhat_df)))
    return std::nullopt;

  return tangent;
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint>
PorousViscoplasticityStressUpdateTempl<is_ad>::recoverReducedPorosityRootBeforeFold(
    ReducedPorosityBracket & bracket,
    const LocalPoint & fold_upper,
    const LocalSolveContext & context,
    Real & best_abs_rf_scaled)
{
  using std::abs;
  using std::max;
  using std::min;

  auto tangent_lower_point = bracket.lower;
  auto tangent_upper_point = fold_upper;
  const auto tangent_lower = computeReducedPorosityTangent(tangent_lower_point.jacobian);
  const auto tangent_upper = computeReducedPorosityTangent(tangent_upper_point.jacobian);
  if (!tangent_lower || !tangent_upper)
    return std::nullopt;

  auto slope_lower = MetaPhysicL::raw_value(tangent_lower->drhat_df);
  auto slope_upper = MetaPhysicL::raw_value(tangent_upper->drhat_df);
  if (!(slope_lower > 0.0) || !(slope_upper <= 0.0))
    return std::nullopt;

  auto best_fold_point =
      abs(slope_lower) <= abs(slope_upper) ? tangent_lower_point : tangent_upper_point;
  auto best_fold_slope = abs(slope_lower) <= abs(slope_upper) ? slope_lower : slope_upper;

  const auto record_root_bracket = [&](const LocalPoint & lower, const LocalPoint & upper)
  {
    bracket.lower = lower;
    bracket.upper = upper;
  };

  const auto fold_tolerance = max(_porosity_bound_tolerance,
                                  1.0e-8 * max({abs(MetaPhysicL::raw_value(tangent_lower_point.f)),
                                                abs(MetaPhysicL::raw_value(tangent_upper_point.f)),
                                                _minimum_porosity}));

  for (auto iteration = 0u; iteration < _reduced_porosity_root_max_iterations; ++iteration)
  {
    const auto lower_f = MetaPhysicL::raw_value(tangent_lower_point.f);
    const auto upper_f = MetaPhysicL::raw_value(tangent_upper_point.f);
    const auto width = upper_f - lower_f;
    if (!(width > fold_tolerance))
      break;

    auto fraction = slope_lower / (slope_lower - slope_upper);
    if (!std::isfinite(fraction))
      fraction = 0.5;
    const auto minimum_fraction = min(0.1, max(1.0e-6, fold_tolerance / width));
    fraction = min(1.0 - minimum_fraction, max(minimum_fraction, fraction));

    auto f_probe = lower_f + fraction * width;
    auto probe_seed = LocalCoordinates{
        tangent_lower_point.p + fraction * (tangent_upper_point.p - tangent_lower_point.p),
        tangent_lower_point.q + fraction * (tangent_upper_point.q - tangent_lower_point.q),
        f_probe};

    auto probe_result = solveReducedPoint(probe_seed, context, best_abs_rf_scaled);

    if (!probe_result)
    {
      fraction = 0.5;
      f_probe = lower_f + fraction * width;
      probe_seed = LocalCoordinates{0.5 * (tangent_lower_point.p + tangent_upper_point.p),
                                    0.5 * (tangent_lower_point.q + tangent_upper_point.q),
                                    f_probe};
      probe_result = solveReducedPoint(probe_seed, context, best_abs_rf_scaled);
      if (!probe_result)
        return std::nullopt;
    }

    const auto & probe_point = *probe_result;
    const auto residual = reducedPorosityResidual(probe_point);
    if (!std::isfinite(residual))
      return std::nullopt;

    if (reducedPointConverged(probe_point, context))
    {
      return probe_point;
    }

    const auto probe_tangent = computeReducedPorosityTangent(probe_point.jacobian);
    if (!probe_tangent)
      return std::nullopt;

    const auto probe_slope = MetaPhysicL::raw_value(probe_tangent->drhat_df);

    if (abs(probe_slope) < abs(best_fold_slope))
    {
      best_fold_point = probe_point;
      best_fold_slope = probe_slope;
    }

    if (this->_verbose)
      Moose::out << "Reduced porosity fold search: iteration = " << iteration
                 << " f = " << MetaPhysicL::raw_value(probe_point.f)
                 << " Rf/scale = " << residual / context.porosity_convergence_scale
                 << " dRhat_f/df = " << probe_slope << std::endl;

    if (residual >= 0.0)
    {
      record_root_bracket(tangent_lower_point, probe_point);
      return std::nullopt;
    }

    if (probe_slope > 0.0)
    {
      tangent_lower_point = probe_point;
      slope_lower = probe_slope;
    }
    else
    {
      tangent_upper_point = probe_point;
      slope_upper = probe_slope;
    }
  }

  const auto fold_residual = reducedPorosityResidual(best_fold_point);

  if (this->_verbose)
    Moose::out << "Reduced porosity fold search: local maximum estimate f = "
               << MetaPhysicL::raw_value(best_fold_point.f)
               << " Rf/scale = " << fold_residual / context.porosity_convergence_scale
               << " dRhat_f/df = " << best_fold_slope
               << (fold_residual >= 0.0 ? "; lower root is bracketed."
                                        : "; maximum remains below zero.")
               << std::endl;

  if (fold_residual >= 0.0 &&
      MetaPhysicL::raw_value(best_fold_point.f) > MetaPhysicL::raw_value(tangent_lower_point.f))
    record_root_bracket(tangent_lower_point, best_fold_point);

  return std::nullopt;
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalPoint>
PorousViscoplasticityStressUpdateTempl<is_ad>::discoverReducedPorosityBracket(
    ReducedPorosityBracket & bracket, const LocalSolveContext & context, Real & best_abs_rf_scaled)
{
  using std::max;
  using std::min;
  using std::sqrt;

  const auto porosity_begin = MetaPhysicL::raw_value(context.porosity_begin);
  const auto initial_f = MetaPhysicL::raw_value(bracket.lower.f);
  const auto initial_distance = max({_minimum_porosity,
                                     10.0 * _porosity_bound_tolerance,
                                     porosity_begin - _minimum_porosity,
                                     initial_f - _minimum_porosity,
                                     1.0e-16});
  const auto search_target = reducedPorositySearchTarget();
  constexpr auto maximum_probe_porosity = Real(0.95);

  /*
   * Search upward by continuation from the last mechanically equilibrated point. Use the exact
   * Schur-complement tangent to predict the reduced root, then bound that prediction by geometric
   * growth and an optional model-specific waypoint. This avoids the implicit unit-tangent
   * assumption in f-Rf, which becomes unreliable under strong pore-growth feedback.
   *
   * A failed fixed-f mechanical solve defines an upper continuation boundary. Backtrack between
   * that failed target and the last successful lower point rather than expanding farther away.
   */
  std::optional<Real> failed_probe;
  auto retry_failed_probe = false;

  const auto continuation_midpoint = [&](const Real lower, const Real upper)
  {
    if (lower > 0.0 && upper / lower > 4.0)
      return sqrt(lower * upper);
    return 0.5 * (lower + upper);
  };

  for (auto probe = 0u; probe < _reduced_porosity_max_probes; ++probe)
  {
    const auto lower_f = MetaPhysicL::raw_value(bracket.lower.f);
    auto f_probe = lower_f;
    auto probing_failed_target = false;

    if (failed_probe)
    {
      probing_failed_target = retry_failed_probe;
      f_probe =
          probing_failed_target ? *failed_probe : continuation_midpoint(lower_f, *failed_probe);
    }
    else
    {
      const auto tangent = computeReducedPorosityTangent(bracket.lower.jacobian);
      if (!tangent || MetaPhysicL::raw_value(tangent->drhat_df) <= 0.0)
      {
        if (this->_verbose)
          Moose::out << "Reduced porosity continuation stopped: invalid or nonpositive "
                        "dRhat_f/df. Requesting timestep cut."
                     << std::endl;
        return std::nullopt;
      }

      const auto drhat_df = MetaPhysicL::raw_value(tangent->drhat_df);
      const auto lower_residual = reducedPorosityResidual(bracket.lower);
      const auto f_newton = bracket.lower.f - lower_residual / tangent->drhat_df;
      const auto newton_target = MetaPhysicL::raw_value(f_newton);
      if (!std::isfinite(newton_target) || !(newton_target > lower_f))
      {
        if (this->_verbose)
          Moose::out << "Reduced porosity continuation stopped: Schur-Newton target does not "
                        "advance on the positive-slope branch. lower f = "
                     << lower_f << " target f = " << newton_target << " dRhat_f/df = " << drhat_df
                     << std::endl;
        return std::nullopt;
      }

      const auto distance_from_floor = max(lower_f - _minimum_porosity, initial_distance);
      const auto trust_target =
          min(_minimum_porosity + _reduced_porosity_probe_growth * distance_from_floor,
              maximum_probe_porosity);
      f_probe = min(newton_target, trust_target);
      if (search_target && *search_target > lower_f && *search_target < f_probe)
      {
        f_probe = *search_target;
        if (this->_verbose)
          Moose::out << "Reduced porosity continuation: model-specific waypoint f = " << f_probe
                     << std::endl;
      }
    }

    if (!(f_probe > lower_f))
      break;

    const auto probe_seed = LocalCoordinates{bracket.lower.p, bracket.lower.q, f_probe};

    const auto probe_result = solveReducedPoint(probe_seed, context, best_abs_rf_scaled);
    if (!probe_result)
    {
      if (!probing_failed_target)
        failed_probe = f_probe;
      retry_failed_probe = false;

      if (this->_verbose)
        Moose::out << "Reduced porosity probe failed; backtracking:"
                   << " lower f = " << lower_f << " failed f = " << f_probe << std::endl;

      continue;
    }

    const auto & probe_point = *probe_result;
    const auto residual = reducedPorosityResidual(probe_point);
    if (!std::isfinite(residual))
      return std::nullopt;

    if (this->_verbose)
      Moose::out << "Reduced porosity probe:"
                 << " f = " << MetaPhysicL::raw_value(probe_point.f)
                 << " p = " << MetaPhysicL::raw_value(probe_point.p)
                 << " q = " << MetaPhysicL::raw_value(probe_point.q)
                 << " Rf/scale = " << residual / context.porosity_convergence_scale << std::endl;

    if (reducedPointConverged(probe_point, context))
      return probe_point;

    if (residual >= 0.0)
    {
      bracket.upper = probe_point;
      return std::nullopt;
    }

    /*
     * A positive-to-nonpositive Schur slope means the reduced residual has a local maximum between
     * the last negative-residual point and this probe. Locate that fold instead of immediately
     * requesting a global timestep cut. If Rhat_f reaches zero before the maximum, the lower FREE
     * root is continuation-connected and can be solved with the ordinary reduced bracket solver.
     */
    const auto lower_tangent = computeReducedPorosityTangent(bracket.lower.jacobian);
    const auto probe_tangent = computeReducedPorosityTangent(probe_point.jacobian);
    if (lower_tangent && probe_tangent && MetaPhysicL::raw_value(lower_tangent->drhat_df) > 0.0 &&
        MetaPhysicL::raw_value(probe_tangent->drhat_df) <= 0.0)
    {
      if (const auto root = recoverReducedPorosityRootBeforeFold(
              bracket, probe_point, context, best_abs_rf_scaled))
        return root;

      return std::nullopt;
    }

    bracket.lower = probe_point;
    if (failed_probe)
    {
      if (probing_failed_target)
      {
        failed_probe.reset();
        retry_failed_probe = false;
      }
      else
        retry_failed_probe = true;
    }
  }

  return std::nullopt;
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalSolveResult>
PorousViscoplasticityStressUpdateTempl<is_ad>::solveReducedPorosityBracket(
    ReducedPorosityBracket & bracket, const LocalSolveContext & context, Real & best_abs_rf_scaled)
{
  using std::abs;
  using std::min;

  mooseAssert(bracket.upper, "Reduced porosity root solve requires a complete bracket.");

  /*
   * porosity_bound_tolerance is an active-set tolerance, not by itself a sufficiently tight root
   * tolerance when Rhat_f is steep. Also require the bracket error to be small relative to the
   * scaled porosity residual tolerance before accepting width as the convergence certificate.
   */
  const auto bracket_tolerance =
      min(_porosity_bound_tolerance,
          0.1 * context.porosity_convergence_scale * _local_newton_stagnation_tolerance);

  const auto update_best = [&](const LocalPoint & candidate)
  {
    const auto scaled_porosity_residual =
        abs(reducedPorosityResidual(candidate)) / context.porosity_convergence_scale;
    best_abs_rf_scaled = std::min(best_abs_rf_scaled, scaled_porosity_residual);
  };

  const auto converged_result = [&](const LocalPoint & candidate, const char * reason)
  {
    if (this->_verbose)
      Moose::out << "Reduced porosity root: converged by " << reason
                 << ". f = " << MetaPhysicL::raw_value(candidate.f) << " |Rf|/scale = "
                 << abs(reducedPorosityResidual(candidate)) / context.porosity_convergence_scale
                 << std::endl;

    return LocalSolveResult{candidate};
  };

  const auto bracket_converged_result =
      [&](const Real bracket_width) -> std::optional<LocalSolveResult>
  {
    const auto lower_residual = abs(reducedPorosityResidual(bracket.lower));
    const auto upper_residual = abs(reducedPorosityResidual(*bracket.upper));
    const auto & candidate = lower_residual <= upper_residual ? bracket.lower : *bracket.upper;

    const auto polished = solveMechanicalAtFixedPorosity(
        candidate.coordinates(), context.reduced_final_tolerance, context);
    if (!polished)
      return std::nullopt;

    update_best(*polished);
    if (reducedPointConverged(*polished, context))
      return converged_result(*polished, "residual after bracket polish");

    if (this->_verbose)
      Moose::out << "Reduced porosity root: converged by bracket width. f = "
                 << MetaPhysicL::raw_value(polished->f)
                 << " porosity error bound = " << bracket_width << " |Rf|/scale = "
                 << abs(reducedPorosityResidual(*polished)) / context.porosity_convergence_scale
                 << std::endl;

    return LocalSolveResult{*polished, false, bracket_width};
  };

  /* Tighten both bracket endpoints before treating their signs as the reduced function. */
  const auto tight_lower = solveMechanicalAtFixedPorosity(
      bracket.lower.coordinates(), context.reduced_final_tolerance, context);
  const auto tight_upper = solveMechanicalAtFixedPorosity(
      bracket.upper->coordinates(), context.reduced_final_tolerance, context);
  if (!tight_lower || !tight_upper)
    return std::nullopt;

  bracket.lower = *tight_lower;
  bracket.upper = *tight_upper;
  update_best(bracket.lower);
  update_best(*bracket.upper);

  if (reducedPointConverged(bracket.lower, context))
    return converged_result(bracket.lower, "residual");
  if (reducedPointConverged(*bracket.upper, context))
    return converged_result(*bracket.upper, "residual");

  auto lower_residual = reducedPorosityResidual(bracket.lower);
  auto upper_residual = reducedPorosityResidual(*bracket.upper);
  if (lower_residual >= 0.0 || upper_residual <= 0.0)
    return std::nullopt;

  /*
   * Use the Schur predictor while it is productive. Near the residual floor, force midpoint
   * contraction so the sign-changing bracket itself provides a robust termination certificate.
   */
  for (auto root_iteration = 0u; root_iteration < _reduced_porosity_root_max_iterations;
       ++root_iteration)
  {
    const auto & upper = *bracket.upper;
    const auto f_lower = MetaPhysicL::raw_value(bracket.lower.f);
    const auto f_upper = MetaPhysicL::raw_value(upper.f);
    const auto bracket_width = f_upper - f_lower;
    if (bracket_width <= 0.0)
      return std::nullopt;

    if (bracket_width <= bracket_tolerance)
      return bracket_converged_result(bracket_width);

    lower_residual = reducedPorosityResidual(bracket.lower);
    upper_residual = reducedPorosityResidual(upper);
    const auto use_lower = abs(lower_residual) <= abs(upper_residual);
    const auto & base = use_lower ? bracket.lower : upper;

    auto mid_coordinates = reducedBracketMidpoint(bracket.lower, upper);
    auto used_newton_predictor = false;

    const auto near_residual_floor =
        best_abs_rf_scaled <= 10.0 * _local_newton_stagnation_tolerance;
    if (!near_residual_floor)
    {
      const auto tangent = computeReducedPorosityTangent(base.jacobian);
      if (tangent)
      {
        const auto f_newton = base.f - base.residual[F_INDEX] / tangent->drhat_df;
        const auto f_newton_raw = MetaPhysicL::raw_value(f_newton);
        const auto margin = 0.1 * bracket_width;

        if (std::isfinite(f_newton_raw) && f_newton_raw > f_lower + margin &&
            f_newton_raw < f_upper - margin)
        {
          mid_coordinates.f = f_newton;
          const auto delta_f = mid_coordinates.f - base.f;
          mid_coordinates.p = base.p + tangent->dp_df * delta_f;
          mid_coordinates.q = base.q + tangent->dq_df * delta_f;
          used_newton_predictor = true;
        }
      }
    }

    auto mid_result = solveReducedPoint(mid_coordinates, context, best_abs_rf_scaled);

    if (!mid_result && used_newton_predictor)
    {
      mid_coordinates = reducedBracketMidpoint(bracket.lower, upper);
      mid_result = solveReducedPoint(mid_coordinates, context, best_abs_rf_scaled);
      used_newton_predictor = false;
    }

    if (!mid_result)
      return std::nullopt;

    const auto & mid = *mid_result;
    const auto residual = reducedPorosityResidual(mid);

    if (this->_verbose)
      Moose::out << "Reduced porosity root:"
                 << " iteration = " << root_iteration << " f = " << MetaPhysicL::raw_value(mid.f)
                 << " Rf/scale = " << residual / context.porosity_convergence_scale
                 << " bracket width = " << bracket_width
                 << " used Newton = " << used_newton_predictor << std::endl;

    if (reducedPointConverged(mid, context))
      return converged_result(mid, "residual");

    if (residual < 0.0)
      bracket.lower = mid;
    else
      bracket.upper = mid;
  }

  const auto final_width = MetaPhysicL::raw_value(bracket.upper->f - bracket.lower.f);
  if (final_width > 0.0 && final_width <= bracket_tolerance)
    return bracket_converged_result(final_width);

  return std::nullopt;
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalSolveResult>
PorousViscoplasticityStressUpdateTempl<is_ad>::solveReducedPorosityDownward(
    LocalPoint upper, const LocalSolveContext & context, Real & best_abs_rf_scaled)
{
  using std::max;
  using std::min;
  using std::sqrt;

  const auto porosity_begin =
      max(_minimum_porosity, MetaPhysicL::raw_value(context.porosity_begin));

  const auto downward_target = [&](const LocalPoint & positive_point)
  {
    const auto current_f = MetaPhysicL::raw_value(positive_point.f);
    const auto lower_waypoint =
        current_f > porosity_begin + _porosity_bound_tolerance ? porosity_begin : _minimum_porosity;
    const auto implied_f = MetaPhysicL::raw_value(impliedPorosity(positive_point));

    if (!std::isfinite(implied_f) || implied_f >= current_f - _porosity_bound_tolerance)
      return lower_waypoint;

    return max(lower_waypoint, implied_f);
  };

  auto target_f = downward_target(upper);
  for (auto probe = 0u; probe < _reduced_porosity_max_probes; ++probe)
  {
    const auto current_f = MetaPhysicL::raw_value(upper.f);
    if (current_f <= _minimum_porosity + _porosity_bound_tolerance)
      return LocalSolveResult{upper, true};

    target_f = max(_minimum_porosity, min(target_f, current_f));
    if (target_f >= current_f - _porosity_bound_tolerance)
      target_f = _minimum_porosity;

    auto f_probe = target_f;
    if (target_f > 0.0 && current_f / target_f > 4.0)
      f_probe = sqrt(current_f * target_f);

    const auto probe_seed = LocalCoordinates{upper.p, upper.q, f_probe};
    const auto probe_result = solveReducedPoint(probe_seed, context, best_abs_rf_scaled);

    if (!probe_result)
    {
      /* Backtrack toward the last mechanically solved positive-residual point. */
      target_f = f_probe > 0.0 && current_f / f_probe > 4.0 ? sqrt(current_f * f_probe)
                                                            : 0.5 * (current_f + f_probe);
      continue;
    }

    const auto & probe_point = *probe_result;
    const auto residual = reducedPorosityResidual(probe_point);

    if (this->_verbose)
      Moose::out << "Reduced porosity downward probe:"
                 << " f = " << MetaPhysicL::raw_value(probe_point.f)
                 << " p = " << MetaPhysicL::raw_value(probe_point.p)
                 << " q = " << MetaPhysicL::raw_value(probe_point.q)
                 << " Rf/scale = " << residual / context.porosity_convergence_scale << std::endl;

    if (reducedPointConverged(probe_point, context))
      return LocalSolveResult{probe_point};

    if (residual <= 0.0)
    {
      auto bracket = ReducedPorosityBracket(probe_point);
      bracket.upper = upper;
      return solveReducedPorosityBracket(bracket, context, best_abs_rf_scaled);
    }

    upper = probe_point;
    if (upper.f <= _minimum_porosity + _porosity_bound_tolerance)
      return LocalSolveResult{upper, true};

    target_f = downward_target(upper);
  }

  return std::nullopt;
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalSolveResult>
PorousViscoplasticityStressUpdateTempl<is_ad>::solveReducedPorosityUpward(
    const LocalPoint & lower, const LocalSolveContext & context, Real & best_abs_rf_scaled)
{
  const auto lower_residual = reducedPorosityResidual(lower);
  const auto initial_tangent = computeReducedPorosityTangent(lower.jacobian);
  const auto nan = std::numeric_limits<Real>::quiet_NaN();
  const auto initial_dp_df = initial_tangent ? MetaPhysicL::raw_value(initial_tangent->dp_df) : nan;
  const auto initial_dq_df = initial_tangent ? MetaPhysicL::raw_value(initial_tangent->dq_df) : nan;
  const auto initial_drhat_df =
      initial_tangent ? MetaPhysicL::raw_value(initial_tangent->drhat_df) : nan;
  auto initial_newton_target_f = nan;
  if (initial_tangent && initial_drhat_df != 0.0)
    initial_newton_target_f =
        MetaPhysicL::raw_value(lower.f - lower_residual / initial_tangent->drhat_df);

  if (this->_verbose)
    Moose::out << "Reduced porosity tangent:"
               << " valid = " << initial_tangent.has_value() << " dp/df = " << initial_dp_df
               << " dq/df = " << initial_dq_df << " dRhat_f/df = " << initial_drhat_df
               << " Newton target f = " << initial_newton_target_f << std::endl;

  auto bracket = ReducedPorosityBracket(lower);
  if (!initial_tangent || initial_drhat_df <= 0.0)
  {
    using std::max;
    using std::min;

    const auto lower_f = MetaPhysicL::raw_value(lower.f);
    const auto porosity_begin = MetaPhysicL::raw_value(context.porosity_begin);
    const auto initial_distance = max({_minimum_porosity,
                                       10.0 * _porosity_bound_tolerance,
                                       porosity_begin - _minimum_porosity,
                                       lower_f - _minimum_porosity,
                                       1.0e-16});
    const auto trust_target =
        min(_minimum_porosity + _reduced_porosity_probe_growth * initial_distance, Real(0.95));
    const auto search_target = reducedPorositySearchTarget();

    auto probe_f = trust_target;
    if (search_target && *search_target > lower_f && *search_target < probe_f)
      probe_f = *search_target;

    if (this->_verbose)
    {
      Moose::out << "Reduced porosity tangent is nonpositive or invalid; trying one bounded "
                    "upward diagnostic probe before requesting a timestep cut. lower f = "
                 << lower_f << " probe f = " << probe_f << " trust target f = " << trust_target;
      if (search_target)
        Moose::out << " model-specific target f = " << *search_target;
      Moose::out << std::endl;
    }

    if (!(probe_f > lower_f + _porosity_bound_tolerance))
      return std::nullopt;

    const auto probe_seed = LocalCoordinates{lower.p, lower.q, probe_f};
    const auto probe_result = solveReducedPoint(probe_seed, context, best_abs_rf_scaled);
    if (!probe_result)
      return std::nullopt;

    const auto & probe_point = *probe_result;
    const auto probe_residual = reducedPorosityResidual(probe_point);
    if (!std::isfinite(probe_residual))
      return std::nullopt;

    if (this->_verbose)
      Moose::out << "Reduced porosity fold probe: f = " << MetaPhysicL::raw_value(probe_point.f)
                 << " Rf/scale = " << probe_residual / context.porosity_convergence_scale
                 << std::endl;

    if (reducedPointConverged(probe_point, context))
      return LocalSolveResult{probe_point};

    if (probe_residual >= 0.0)
    {
      bracket.upper = probe_point;
      return solveReducedPorosityBracket(bracket, context, best_abs_rf_scaled);
    }

    const auto probe_tangent = computeReducedPorosityTangent(probe_point.jacobian);
    if (!probe_tangent || MetaPhysicL::raw_value(probe_tangent->drhat_df) <= 0.0)
      return std::nullopt;

    bracket.lower = probe_point;
  }

  if (const auto root = discoverReducedPorosityBracket(bracket, context, best_abs_rf_scaled))
    return LocalSolveResult{*root};

  if (!bracket.complete())
    return std::nullopt;

  return solveReducedPorosityBracket(bracket, context, best_abs_rf_scaled);
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalSolveResult>
PorousViscoplasticityStressUpdateTempl<is_ad>::solveReducedPorosity(
    const LocalPoint & point, const LocalSolveContext & context, bool & reduced_porosity_attempted)
{
  reduced_porosity_attempted = true;
  auto best_abs_rf_scaled = std::numeric_limits<Real>::infinity();

  /* First solve Rp = Rq = 0 at the current f before treating Rf as the reduced residual. */
  auto lower_result = solveReducedPoint(point.coordinates(), context, best_abs_rf_scaled);

  /*
   * At the actual floor the trial stress can occasionally be a better seed than a badly wandered
   * full-Newton state.
   */
  if (!lower_result)
  {
    const auto trial_seed = LocalCoordinates{context.p_trial, context.trial_equiv_stress, point.f};
    lower_result = solveReducedPoint(trial_seed, context, best_abs_rf_scaled);
  }

  if (!lower_result)
    return std::nullopt;

  const auto & lower = *lower_result;
  if (reducedPointConverged(lower, context))
    return LocalSolveResult{lower};

  const auto lower_residual = reducedPorosityResidual(lower);
  if (lower.f <= _minimum_porosity + _porosity_bound_tolerance &&
      lower_residual > _porosity_bound_tolerance)
    return LocalSolveResult{lower, true};

  /*
   * The failed coupled point is not on Rp = Rq = 0, so only the mechanically equilibrated Rhat_f
   * determines the search direction. Positive Rhat_f searches downward toward the accepted
   * porosity/floor; negative Rhat_f continues upward along the mechanically equilibrated manifold.
   */
  return lower_residual > 0.0 ? solveReducedPorosityDownward(lower, context, best_abs_rf_scaled)
                              : solveReducedPorosityUpward(lower, context, best_abs_rf_scaled);
}

template <bool is_ad>
bool
PorousViscoplasticityStressUpdateTempl<is_ad>::adaptiveSubstepRefinementAvailable() const
{
  if (!this->_adaptive_substepping || this->_maximum_number_substeps <= 1)
    return false;

  const auto global_dt = this->globalTimeStep();
  const auto constitutive_dt = this->constitutiveTimeStep();
  if (!std::isfinite(global_dt) || !std::isfinite(constitutive_dt) || global_dt <= 0.0 ||
      constitutive_dt <= 0.0)
    return false;

  const auto minimum_constitutive_dt = global_dt / this->_maximum_number_substeps;
  return constitutive_dt > minimum_constitutive_dt * (1.0 + 1.0e-12);
}

template <bool is_ad>
std::optional<typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalSolveResult>
PorousViscoplasticityStressUpdateTempl<is_ad>::tryReducedPorosityRecovery(
    const LocalPoint & point, const LocalSolveContext & context, bool & reduced_porosity_attempted)
{
  if (point.porosityFloorActive() || reduced_porosity_attempted)
    return std::nullopt;

  auto reduced = solveReducedPorosity(point, context, reduced_porosity_attempted);
  if (!reduced)
    return std::nullopt;

  if (!reduced->activate_floor)
    reduced->point = verifyReducedConvergedPoint(*reduced, context);

  return reduced;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalSolveResult
PorousViscoplasticityStressUpdateTempl<is_ad>::recoverFailedCoupledLineSearch(
    const LocalPoint & point, const LocalSolveContext & context, bool & reduced_porosity_attempted)
{
  /*
   * Reduced-porosity globalization is intentionally deferred while the MOOSE adaptive substepper
   * can still reduce the constitutive timestep. The reduced solve can require many additional
   * p-q equilibrations and porosity probes; paying that cost on a coarse local attempt is wasted if
   * the parent driver can simply retry with a finer subdivision. At the finest allowed subdivision
   * retain the full reduced-porosity rescue before requesting a global timestep cut.
   */
  const auto adaptive_refinement_available = adaptiveSubstepRefinementAvailable();

  if (!adaptive_refinement_available && !reduced_porosity_attempted)
  {
    if (const auto recovered =
            tryReducedPorosityRecovery(point, context, reduced_porosity_attempted))
      return *recovered;

    if (this->_verbose && reduced_porosity_attempted)
      Moose::out << "Reduced porosity search did not find a converged free root."
                 << " f = " << MetaPhysicL::raw_value(point.f) << " Rf/scale = "
                 << MetaPhysicL::raw_value(point.residual[F_INDEX]) /
                        context.porosity_convergence_scale
                 << std::endl;
  }

  /* Activate the physical floor only when the free increment itself reaches the bound. */
  if (freeIncrementReachesPorosityFloor(point))
    return {point, true};

  const auto residual_norm = convergenceResidualNorm(point.residual, context);
  if (residual_norm <= _local_newton_stagnation_tolerance)
    return {verifyConvergedPoint(point, context), false};

  if (adaptive_refinement_available)
  {
    if (this->_verbose)
      Moose::out << "In " << this->_name
                 << ": deferring reduced-porosity recovery to finer adaptive substeps."
                 << std::endl;
    mooseException("In ",
                   this->_name,
                   ": coupled local line search failed while finer adaptive constitutive "
                   "substeps remain available.");
  }

  throwCoupledLineSearchFailure(point, context, reduced_porosity_attempted);
}

template <bool is_ad>
[[noreturn]] void
PorousViscoplasticityStressUpdateTempl<is_ad>::throwCoupledLineSearchFailure(
    const LocalPoint & point,
    const LocalSolveContext & context,
    const bool reduced_porosity_attempted)
{
  const auto residual = scaledResidual(point.residual, context);
  const auto & hydrostatic_stress = point.hydrostatic_stress;
  const auto residual_norm = residualNorm(residual);
  const auto convergence_residual_norm = convergenceResidualNorm(point.residual, context);
  const auto mechanical_residual_norm = physicalMechanicalResidualNorm(
      point.residual, context.p_trial, context.trial_equiv_stress, this->_minimum_stress_magnitude);

  mooseException("In ",
                 this->_name,
                 ": analytical coupled local (p,q,f) Newton line search failed on timestep ",
                 this->_t_step,
                 "\n  constitutive dt = ",
                 this->constitutiveTimeStep(),
                 "\n  global dt = ",
                 this->globalTimeStep(),
                 "\n  scaled residual norm = ",
                 residual_norm,
                 "\n  convergence residual norm = ",
                 convergence_residual_norm,
                 "\n  physical mechanical residual norm = ",
                 mechanical_residual_norm,
                 "\n  scaled Rp = ",
                 MetaPhysicL::raw_value(residual[P_INDEX]),
                 "\n  scaled Rq = ",
                 MetaPhysicL::raw_value(residual[Q_INDEX]),
                 "\n  scaled Rf = ",
                 MetaPhysicL::raw_value(residual[F_INDEX]),
                 "\n  physical scaled Rf = ",
                 MetaPhysicL::raw_value(point.residual[F_INDEX]) /
                     context.porosity_convergence_scale,
                 "\n  p scale = ",
                 context.p_scale,
                 "\n  q scale = ",
                 context.q_scale,
                 "\n  porosity residual scale = ",
                 context.porosity_merit_scale,
                 "\n  porosity variable scale = ",
                 context.porosity_variable_scale,
                 "\n  porosity convergence scale = ",
                 context.porosity_convergence_scale,
                 "\n  p = ",
                 MetaPhysicL::raw_value(point.p),
                 "\n  q = ",
                 MetaPhysicL::raw_value(point.q),
                 "\n  f = ",
                 MetaPhysicL::raw_value(point.f),
                 "\n  f_begin = ",
                 MetaPhysicL::raw_value(context.porosity_begin),
                 "\n  f_min = ",
                 _minimum_porosity,
                 "\n  floor active = ",
                 point.porosityFloorActive(),
                 "\n  effective hydrostatic stress = ",
                 MetaPhysicL::raw_value(hydrostatic_stress.effective_hydro_stress),
                 "\n  d(p_eff)/df = ",
                 MetaPhysicL::raw_value(hydrostatic_stress.deffective_hydro_df),
                 "\n  reduced fallback used = ",
                 reduced_porosity_attempted);
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LocalSolveResult
PorousViscoplasticityStressUpdateTempl<is_ad>::solveCoupledNewton(LocalPoint point,
                                                                  const LocalSolveContext & context,
                                                                  bool & reduced_porosity_attempted)
{
  /*
   * The active-set driver supplies a fully evaluated point on the current branch. Every accepted
   * Newton, reduced-porosity, or branch-transition update likewise replaces it with a complete
   * LocalPoint, so coordinates, branch, residual, and Jacobian remain synchronized throughout.
   */
  const auto porosity_branch = point.porosity_branch;

  for (auto iteration = 0u; iteration < _local_newton_max_iterations; ++iteration)
  {
    validateFiniteLocalPoint(point, "coupled Newton iteration");
    const auto residual_norm = convergenceResidualNorm(point.residual, context);
    if (residual_norm <= _local_newton_tolerance)
      return {verifyConvergedPoint(point, context), false};

    const auto jacobian = scaledJacobian(point.jacobian, context);
    const auto residual = scaledResidual(point.residual, context);
    const auto rhs = LocalResidual{-residual[P_INDEX], -residual[Q_INDEX], -residual[F_INDEX]};
    auto correction = LocalResidual{};
    if (!solveLinearSystem(jacobian, rhs, correction))
      mooseException("In ", this->_name, ": singular analytical coupled local (p,q,f) Jacobian.");

    auto alpha = _local_newton_relaxation;
    if (!point.porosityFloorActive())
    {
      const auto delta_f =
          context.porosity_variable_scale * MetaPhysicL::raw_value(correction[F_INDEX]);
      const auto distance_to_floor = MetaPhysicL::raw_value(point.f) - _minimum_porosity;
      if (delta_f < 0.0 && distance_to_floor > 0.0)
        alpha = std::min(alpha, 0.99 * distance_to_floor / (-delta_f));
    }

    const auto candidate =
        backtrackingLineSearch(point, correction, alpha, LocalResidualScope::COUPLED, context);
    if (!candidate)
      return recoverFailedCoupledLineSearch(point, context, reduced_porosity_attempted);

    point = *candidate;
  }

  const auto mechanical_residual_norm =
      residualNorm(scaledResidual(point.residual, context), LocalResidualScope::MECHANICAL);
  const auto residual_norm = convergenceResidualNorm(point.residual, context);

  /*
   * After the iteration limit, activate the free-branch floor only if mechanics are already
   * reasonably equilibrated and the current constitutive increment reaches the bound.
   */
  if (porosity_branch == PorosityBranch::FREE &&
      mechanical_residual_norm <= context.reduced_probe_tolerance &&
      freeIncrementReachesPorosityFloor(point))
    return {point, true};

  if (residual_norm <= _local_newton_stagnation_tolerance)
    return {verifyConvergedPoint(point, context), false};

  /*
   * A coupled Newton solve can exhaust its iteration budget while continuing to accept line-search
   * steps, so the failed-line-search recovery path is never reached. Prefer a cheaper adaptive
   * constitutive refinement while one remains available; only the finest allowed subdivision pays
   * for the reduced-porosity globalization before requesting a global timestep cut.
   */
  if (adaptiveSubstepRefinementAvailable())
  {
    if (this->_verbose)
      Moose::out << "In " << this->_name
                 << ": deferring reduced-porosity recovery after the coupled Newton iteration "
                    "limit to finer adaptive substeps."
                 << std::endl;
    mooseException("In ",
                   this->_name,
                   ": coupled local Newton reached its iteration limit while finer adaptive "
                   "constitutive substeps remain available.");
  }

  if (porosity_branch == PorosityBranch::FREE && !reduced_porosity_attempted)
  {
    if (this->_verbose)
      Moose::out << "Reduced porosity recovery after coupled Newton iteration limit: f = "
                 << MetaPhysicL::raw_value(point.f) << " Rf/scale = "
                 << MetaPhysicL::raw_value(point.residual[F_INDEX]) /
                        context.porosity_convergence_scale
                 << std::endl;

    if (const auto recovered =
            tryReducedPorosityRecovery(point, context, reduced_porosity_attempted))
      return *recovered;
  }

  mooseException("In ",
                 this->_name,
                 ": analytical coupled local (p,q,f) Newton failed to converge in ",
                 _local_newton_max_iterations,
                 " iterations. Final scaled residual norm = ",
                 residual_norm,
                 ", Rp = ",
                 MetaPhysicL::raw_value(point.residual[P_INDEX]) / context.p_scale,
                 ", Rq = ",
                 MetaPhysicL::raw_value(point.residual[Q_INDEX]) / context.q_scale,
                 ", Rf = ",
                 MetaPhysicL::raw_value(point.residual[F_INDEX]) /
                     context.porosity_convergence_scale,
                 ", reduced fallback used = ",
                 reduced_porosity_attempted,
                 ".");

  return {point, false}; // Unreachable after mooseException.
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
  const auto trial_elastic_strain_increment = elastic_strain_increment;
  const auto trial_stress = stress;
  const auto porosity_begin = boundedBeginningPorosity();
  const auto p_trial = trial_stress.trace() / 3.0;
  const auto trial_dev_stress = trial_stress.deviatoric();
  const auto q_trial = this->equivalentStress(trial_dev_stress);

  /* Allow the derived model to prepare any state needed by the local constitutive solve. */
  this->computeStressInitialize(q_trial, elasticity_tensor);

  auto local_context = LocalSolveContext{p_trial,
                                         trial_dev_stress,
                                         q_trial,
                                         porosity_begin,
                                         trial_elastic_strain_increment,
                                         elastic_strain_old,
                                         elasticity_tensor};
  initializeLocalSolveScales(local_context);

  const auto dense_limit = denseLimitActive(local_context);
  auto reduced_porosity_attempted = false;
  auto point = dense_limit ? solveDenseLimit(local_context)
                           : solvePorosityActiveSet(local_context, reduced_porosity_attempted);

  /*
   * The coupled porous Newton solve uses a Real (raw-value) local Jacobian with AD residual
   * corrections. That transports the implicit sensitivity only asymptotically: differentiating the
   * inexact Newton iteration omits dJ terms proportional to the remaining local residual. Project
   * every converged porous AD state onto the exact implicit sensitivity while preserving the
   * converged p, q, and f values. The dense-limit scalar solve performs its own exact sensitivity
   * projection.
   */
  if constexpr (is_ad)
    if (!dense_limit)
      point = reconstructImplicitSensitivity(point, local_context);

  if constexpr (!is_ad)
    if (_compute_consistent_tangent)
      _last_consistent_tangent = computeConsistentTangent(point, local_context);

  commitLocalPoint(point,
                   local_context,
                   elastic_strain_increment,
                   inelastic_strain_increment,
                   stress,
                   effective_inelastic_strain_increment);
  porosityStateAccepted(inelastic_strain_increment, this->_intermediate_porosity);

  if (this->_verbose)
  {
    Moose::out << this->_name << " analytical local solve: "
               << "p = " << MetaPhysicL::raw_value(point.p)
               << " q = " << MetaPhysicL::raw_value(point.q)
               << " f = " << MetaPhysicL::raw_value(point.f) << " dense_limit = " << dense_limit
               << " porosity_floor_active = " << point.porosityFloorActive() << " p_eff = "
               << MetaPhysicL::raw_value(point.hydrostatic_stress.effective_hydro_stress)
               << " constitutive_dt = " << this->constitutiveTimeStep()
               << " global_dt = " << this->globalTimeStep() << " shared_dt = " << this->_dt
               << " numerical_merit_norm = "
               << residualNorm(scaledResidual(point.residual, local_context))
               << " physical_convergence_norm = "
               << convergenceResidualNorm(point.residual, local_context)
               << " porosity_variable_scale = " << local_context.porosity_variable_scale
               << " porosity_merit_scale = " << local_context.porosity_merit_scale
               << " porosity_convergence_scale = " << local_context.porosity_convergence_scale
               << std::endl;
  }
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
  const auto dev_stress = stress.deviatoric();
  const auto equiv_stress = equivalentStress(dev_stress);
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
    const auto gauge_stress =
        computeGaugeStress(equiv_stress, effective_hydro_stress, porosity, law);
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
  const auto estimated_number_substeps = computeRequiredSubsteps(estimated_effective_increment);

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
      const auto gauge_stress =
          computeGaugeStress(equiv_stress, effective_hydro_stress, porosity, law);
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
PorousViscoplasticityStressUpdateTempl<is_ad>::computeRequiredSubsteps(
    const Real effective_inelastic_strain_increment) const
{
  const auto ratio =
      effective_inelastic_strain_increment / (_substep_tolerance * this->_max_inelastic_increment);
  if (ratio <= 1.0)
    return 1;
  if (ratio >= std::numeric_limits<unsigned int>::max())
    return std::numeric_limits<unsigned int>::max();
  return static_cast<unsigned int>(std::ceil(ratio));
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

  auto predicted_substeps = computeRequiredSubsteps(predicted_increment);

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

  const auto bounded_suggestion = std::min(
      computeRequiredSubsteps(total_number_substeps * increment), _maximum_number_substeps);
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
    GenericRankTwoTensor<is_ad> & stress_new,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    const unsigned int total_number_substeps)
{
  mooseAssert(total_number_substeps > 0,
              "PorousViscoplasticityStressUpdate requires at least one local substep.");
  if (total_number_substeps > _maximum_number_substeps)
    mooseException("The number of substeps computed exceeds 'maximum_number_substeps'.");

  this->setConstitutiveTimeStep(this->globalTimeStep() / total_number_substeps);

  GenericReal<is_ad> accumulated_effective_inelastic_strain_increment = 0.0;
  inelastic_strain_increment.zero();

  if (total_number_substeps == 1)
  {
    const GenericRankTwoTensor<is_ad> elastic_strain_old_ad = elastic_strain_old;
    updateStateOneStep(strain_increment,
                       inelastic_strain_increment,
                       stress_new,
                       elasticity_tensor,
                       elastic_strain_old_ad,
                       accumulated_effective_inelastic_strain_increment);
  }
  else
  {
    const auto strain_increment_per_step = strain_increment / total_number_substeps;

    GenericRankTwoTensor<is_ad> sub_elastic_strain_old = elastic_strain_old;
    auto sub_stress_new = elasticity_tensor * sub_elastic_strain_old;

    strain_increment.zero();

    for (auto step = 0u; step < total_number_substeps; ++step)
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
      checkSubstepIncrement(
          sub_effective_inelastic_strain_increment, total_number_substeps, step + 1);

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
  }

  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + accumulated_effective_inelastic_strain_increment;
  _inelastic_strain[_qp] = _inelastic_strain_old[_qp] + inelastic_strain_increment;

  this->computeStressFinalize(inelastic_strain_increment);
  recordEffectiveInelasticStrainRate(accumulated_effective_inelastic_strain_increment);

  // Preserve the historical one-substep ordering: finalize the accepted trial state before the
  // adaptive a-posteriori check requests a transactional retry.
  if (total_number_substeps == 1)
    checkSubstepIncrement(accumulated_effective_inelastic_strain_increment, 1, 1);

  this->resetConstitutiveTimeStep();
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTempl<is_ad>::updateStateSubstep(
    GenericRankTwoTensor<is_ad> & strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericRankTwoTensor<is_ad> & /*rotation_increment*/,
    GenericRankTwoTensor<is_ad> & stress_new,
    const RankTwoTensor & /*stress_old*/,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    bool /*compute_full_tangent_operator*/,
    RankFourTensor & /*tangent_operator*/)
{
  this->resetConstitutiveTimeStep();
  const auto snapshot =
      captureConstitutiveState(strain_increment, inelastic_strain_increment, stress_new);
  const auto restore = [&]()
  { restoreConstitutiveState(snapshot, strain_increment, inelastic_strain_increment, stress_new); };

  // Initialize this model's substep porosity from inelastic increments already computed by other
  // inelastic models. Each successful local p-q-f solve commits the next porosity state directly.
  this->updateIntermediatePorosity(snapshot.strain_increment);

  auto number_substeps = _adaptive_substepping ? estimateAdaptiveNumberSubstepsFromHistory()
                                               : estimateNumberSubsteps(snapshot.stress);

  while (true)
  {
    if (number_substeps > _maximum_number_substeps)
    {
      restore();
      mooseException("In ",
                     _name,
                     ": estimated number of viscoplastic substeps (",
                     number_substeps,
                     ") exceeds maximum_number_substeps (",
                     _maximum_number_substeps,
                     "). Cutting global time step.");
    }

    restore();
    inelastic_strain_increment.zero();
    this->updateIntermediatePorosity(snapshot.strain_increment);
    _suggested_number_substeps = 0;

    try
    {
      updateStateSubstepInternal(strain_increment,
                                 inelastic_strain_increment,
                                 stress_new,
                                 elasticity_tensor,
                                 elastic_strain_old,
                                 number_substeps);
      return;
    }
    catch (...)
    {
      const auto suggested_number_substeps = _suggested_number_substeps;
      restore();

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

  restore();
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
  if (!_gauge_solve_state.law || _gauge_solve_state.porosity <= 0.0)
    return scale;

  const auto & law = *_gauge_solve_state.law;
  const auto & f = _gauge_solve_state.porosity;
  const auto alpha = law.power_factor;
  const auto A = 1.0 + 2.0 * f / 3.0;
  const auto denominator = 1.0 - (1.0 + alpha) * f + alpha * Utility::pow<2>(f);

  auto lambda_q = GenericReal<is_ad>(0.0);
  if (denominator > 0.0)
    lambda_q = effective_trial_stress * sqrt(A / denominator);

  auto lambda_h = GenericReal<is_ad>(0.0);
  const auto hydro = abs(_gauge_solve_state.effective_hydro_stress);
  if (hydro > 0.0)
  {
    const auto n_to_n = pow(law.power, law.power);
    lambda_h = 1.5 * hydro * pow(f / n_to_n, 1.0 / (law.power + 1.0));
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

  const auto M = abs(effective_hydro_stress) / trial_gauge;
  const auto dM_dtrial_gauge = -M / trial_gauge;
  auto residual_left = Utility::pow<2>(equiv_stress / trial_gauge);
  auto dresidual_left_dtrial_gauge = -2.0 * residual_left / trial_gauge;
  const auto spherical_factor = 1.0 + porosity / 1.5;
  residual_left *= spherical_factor;
  dresidual_left_dtrial_gauge *= spherical_factor;
  const auto h = computeH(law.power, M);
  const auto dh_dM = computeH(law.power, M, true);

  const auto residual = residual_left + porosity * (h + law.power_factor / h) - 1.0 -
                        law.power_factor * Utility::pow<2>(porosity);
  const auto dresidual_dh = porosity * (1.0 - law.power_factor / Utility::pow<2>(h));
  derivative = dresidual_left_dtrial_gauge + dresidual_dh * dh_dM * dM_dtrial_gauge;

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

  const auto mod = pow(1.5 * M, (n + 1.0) / n);

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

  LpsDerivatives d;

  const auto n = law.power;
  const auto alpha = law.power_factor;
  constexpr auto beta = 1.5;

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

  const auto A = 1.0 + 2.0 * f / 3.0;
  constexpr auto dA_df = 2.0 / 3.0;
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
typename PorousViscoplasticityStressUpdateTempl<is_ad>::LpsCreepResponse
PorousViscoplasticityStressUpdateTempl<is_ad>::evaluateLpsCreepResponse(
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & equiv_stress,
    const GenericRankTwoTensor<is_ad> & dev_direction,
    const GenericReal<is_ad> & porosity)
{
  using std::abs;

  auto response = LpsCreepResponse{};
  response.inelastic_strain_increment.zero();
  response.dinelastic_deffective_hydro_stress.zero();
  response.dinelastic_dequiv_stress.zero();
  response.dinelastic_dporosity.zero();

  const auto dev_stress = dev_direction * equiv_stress;
  const auto A = 1.0 + 2.0 * porosity / 3.0;
  constexpr auto dA_df = 2.0 / 3.0;

  for (auto law_index = std::size_t{0}; law_index < _creep_laws.size(); ++law_index)
  {
    const auto coefficient = creepCoefficient(law_index);
    if (MetaPhysicL::raw_value(coefficient) == 0.0)
      continue;

    const auto & law = _creep_laws[law_index];
    const auto gauge_stress =
        computeGaugeStress(equiv_stress, effective_hydro_stress, porosity, law);

    const auto lps =
        computeLpsDerivatives(gauge_stress, effective_hydro_stress, equiv_stress, porosity, law);
    const auto F_lambda_raw = MetaPhysicL::raw_value(lps.F_lambda);
    const auto scaled_F_lambda_raw = MetaPhysicL::raw_value(gauge_stress * lps.F_lambda);
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

    const auto dgauge_dhydro_stress = -lps.F_p / lps.F_lambda;
    const auto dgauge_dequiv_stress = -lps.F_q / lps.F_lambda;
    const auto dgauge_dporosity = -lps.F_f / lps.F_lambda;

    const auto dev_factor = 3.0 * A / Utility::pow<2>(gauge_stress);
    const auto B = _identity_two * (lps.F_p / 3.0) + dev_stress * dev_factor;
    const auto creep_rate = computeCreepRate(law, coefficient, gauge_stress);
    const auto W = -this->constitutiveTimeStep() * creep_rate / lps.F_lambda;

    response.inelastic_strain_increment += B * W;
    response.effective_inelastic_strain_increment += creep_rate * this->constitutiveTimeStep();

    const auto dev_factor_lambda = -2.0 * dev_factor / gauge_stress;
    const auto dev_factor_f = 3.0 * dA_df / Utility::pow<2>(gauge_stress);

    const auto accumulate_derivative = [&](const auto & dgauge_dx,
                                           const Real dp_dx,
                                           const Real dq_dx,
                                           const Real df_dx,
                                           auto & derivative)
    {
      const auto dFp_dx = lps.F_lambdap * dgauge_dx + lps.F_pp * dp_dx + lps.F_pf * df_dx;
      const auto dFlambda_dx = lps.F_lambdalambda * dgauge_dx + lps.F_lambdap * dp_dx +
                               lps.F_lambdaq * dq_dx + lps.F_lambdaf * df_dx;
      const auto ddev_factor_dx = dev_factor_lambda * dgauge_dx + dev_factor_f * df_dx;
      const auto ddev_stress_dx = dev_direction * dq_dx;
      const auto dB_dx = _identity_two * (dFp_dx / 3.0) + dev_stress * ddev_factor_dx +
                         ddev_stress_dx * dev_factor;
      const auto dW_dx = W * (law.power * dgauge_dx / gauge_stress - dFlambda_dx / lps.F_lambda);

      derivative += B * dW_dx + dB_dx * W;
    };

    accumulate_derivative(
        dgauge_dhydro_stress, 1.0, 0.0, 0.0, response.dinelastic_deffective_hydro_stress);
    accumulate_derivative(dgauge_dequiv_stress, 0.0, 1.0, 0.0, response.dinelastic_dequiv_stress);
    accumulate_derivative(dgauge_dporosity, 0.0, 0.0, 1.0, response.dinelastic_dporosity);
  }

  return response;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTempl<is_ad>::computeGaugeStress(
    const GenericReal<is_ad> & equiv_stress,
    const GenericReal<is_ad> & effective_hydro_stress,
    const GenericReal<is_ad> & porosity,
    const CreepLaw & law)
{
  using std::sqrt;

  if (porosity == 0.0)
    return equiv_stress;

  auto gauge_stress = equiv_stress;
  if (effective_hydro_stress == 0.0)
  {
    const auto A = 1.0 + 2.0 * porosity / 3.0;
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
  return gauge_stress;
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
