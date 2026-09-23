//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "PorousViscoplasticityStressUpdateTest.h"

#include "libmesh/utility.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <stdexcept>

registerMooseObject("SolidMechanicsTestApp", PorousViscoplasticityStressUpdateTest);
registerMooseObject("SolidMechanicsTestApp", ADPorousViscoplasticityStressUpdateTest);

namespace
{
bool
rollbackNearlyEqual(const Real left, const Real right)
{
  using std::abs;
  using std::max;

  return abs(left - right) <= 1e-12 * max({1.0, abs(left), abs(right)});
}

bool
rollbackTensorEqual(const RankTwoTensor & left, const RankTwoTensor & right)
{
  for (auto i = 0u; i < 3; ++i)
    for (auto j = 0u; j < 3; ++j)
      if (!rollbackNearlyEqual(left(i, j), right(i, j)))
        return false;

  return true;
}

bool
rollbackTensorEqual(const RankFourTensor & left, const RankFourTensor & right)
{
  for (auto i = 0u; i < 3; ++i)
    for (auto j = 0u; j < 3; ++j)
      for (auto k = 0u; k < 3; ++k)
        for (auto l = 0u; l < 3; ++l)
          if (!rollbackNearlyEqual(left(i, j, k, l), right(i, j, k, l)))
            return false;

  return true;
}
} // namespace

template <bool is_ad>
InputParameters
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::validParams()
{
  InputParameters params = Base::validParams();
  params.addParam<bool>(
      "use_prescribed_two_population_state",
      false,
      "Use two independently evolving pore populations with test-prescribed pressure closures.");
  params.addRangeCheckedParam<Real>(
      "initial_population_0_fraction",
      0.5,
      "initial_population_0_fraction >= 0.0 & initial_population_0_fraction <= 1.0",
      "Initial fraction of total porosity assigned to test pore population 0.");
  params.addRangeCheckedParam<Real>(
      "test_initial_total_porosity",
      0.1,
      "test_initial_total_porosity > 0.0 & test_initial_total_porosity < 1.0",
      "Initial total porosity used only to initialize the test-owned pore-population state.");
  params.addParam<std::vector<Real>>(
      "population_pressures",
      {0.0, 0.0},
      "Constant pressure offsets for test pore populations 0 and 1.");
  params.addParam<std::vector<Real>>(
      "population_pressure_derivatives",
      {0.0, 0.0, 0.0, 0.0},
      "Linear pressure derivatives [dp0/df0, dp0/df1, dp1/df0, dp1/df1] used by the "
      "prescribed two-population closure.");
  params.addParam<std::vector<Real>>(
      "population_porosity_floors",
      {0.0, 0.0},
      "Explicit lower bounds for test pore-population porosities 0 and 1.");
  return params;
}

template <bool is_ad>
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::
    PorousViscoplasticityStressUpdateTestStateTempl(const InputParameters & parameters)
  : Base(parameters),
    _use_prescribed_two_population_state(
        this->template getParam<bool>("use_prescribed_two_population_state")),
    _initial_population_0_fraction(
        this->template getParam<Real>("initial_population_0_fraction")),
    _test_initial_total_porosity(this->template getParam<Real>("test_initial_total_porosity")),
    _population_pressures(this->template getParam<std::vector<Real>>("population_pressures")),
    _population_pressure_derivatives(
        this->template getParam<std::vector<Real>>("population_pressure_derivatives")),
    _population_porosity_floors(
        this->template getParam<std::vector<Real>>("population_porosity_floors")),
    _test_population_0_porosity(
        this->template declareGenericProperty<Real, is_ad>("test_population_0_porosity")),
    _test_population_0_porosity_old(
        this->template getMaterialPropertyOld<Real>("test_population_0_porosity")),
    _test_population_1_porosity(
        this->template declareGenericProperty<Real, is_ad>("test_population_1_porosity")),
    _test_population_1_porosity_old(
        this->template getMaterialPropertyOld<Real>("test_population_1_porosity")),
    _test_population_0_effective_hydrostatic_stress(this->template declareGenericProperty<Real, is_ad>(
        "test_population_0_effective_hydrostatic_stress")),
    _test_population_1_effective_hydrostatic_stress(this->template declareGenericProperty<Real, is_ad>(
        "test_population_1_effective_hydrostatic_stress"))
{
  if (_population_pressures.size() != Base::MAX_HYDROSTATIC_STRESS_POPULATIONS)
    this->paramError("population_pressures", "Exactly two test population pressures are required.");
  if (_population_pressure_derivatives.size() !=
      Base::MAX_HYDROSTATIC_STRESS_POPULATIONS * Base::MAX_HYDROSTATIC_STRESS_POPULATIONS)
    this->paramError("population_pressure_derivatives",
                     "Exactly four pressure derivatives are required.");
  if (_population_porosity_floors.size() != Base::MAX_HYDROSTATIC_STRESS_POPULATIONS)
    this->paramError("population_porosity_floors",
                     "Exactly two test population porosity floors are required.");

  for (const auto pressure : _population_pressures)
    if (!std::isfinite(pressure))
      this->paramError("population_pressures", "Test population pressures must be finite.");
  for (const auto derivative : _population_pressure_derivatives)
    if (!std::isfinite(derivative))
      this->paramError("population_pressure_derivatives",
                       "Test population pressure derivatives must be finite.");
  for (const auto floor : _population_porosity_floors)
    if (!std::isfinite(floor) || floor < 0.0 || floor >= 1.0)
      this->paramError("population_porosity_floors",
                       "Test population porosity floors must be finite and in [0, 1).");
  if (_population_porosity_floors[0] + _population_porosity_floors[1] >= 1.0)
    this->paramError("population_porosity_floors",
                     "The sum of test population porosity floors must be less than one.");
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::initQpStatefulProperties()
{
  Base::initQpStatefulProperties();

  const auto total_porosity = _test_initial_total_porosity;
  _test_population_0_porosity[this->_qp] = _initial_population_0_fraction * total_porosity;
  _test_population_1_porosity[this->_qp] =
      (1.0 - _initial_population_0_fraction) * total_porosity;
  _test_population_0_effective_hydrostatic_stress[this->_qp] = 0.0;
  _test_population_1_effective_hydrostatic_stress[this->_qp] = 0.0;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::propagateQpStatefulProperties()
{
  Base::propagateQpStatefulProperties();
  _test_population_0_porosity[this->_qp] = _test_population_0_porosity_old[this->_qp];
  _test_population_1_porosity[this->_qp] = _test_population_1_porosity_old[this->_qp];
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::resetIncrementalMaterialProperties()
{
  Base::resetIncrementalMaterialProperties();
  _test_population_0_porosity[this->_qp] = _test_population_0_porosity_old[this->_qp];
  _test_population_1_porosity[this->_qp] = _test_population_1_porosity_old[this->_qp];
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::PorePorosityState
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::independentPorePorosityState(
    const GenericReal<is_ad> & total_porosity) const
{
  if (!_use_prescribed_two_population_state)
    return Base::independentPorePorosityState(total_porosity);

  auto state = PorePorosityState{_test_population_0_porosity[this->_qp],
                                 _test_population_1_porosity[this->_qp]};
  const auto stored_total = state[0] + state[1];
  if (stored_total > 0.0)
  {
    const auto scale = total_porosity / stored_total;
    state[0] *= scale;
    state[1] *= scale;
  }
  else
  {
    state[0] = _initial_population_0_fraction * total_porosity;
    state[1] = (1.0 - _initial_population_0_fraction) * total_porosity;
  }
  return state;
}

template <bool is_ad>
GenericReal<is_ad>
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::independentPorePorosityFloor(
    const unsigned int population_index, const PorePorosityState & pore_porosity_begin) const
{
  auto floor = Base::independentPorePorosityFloor(population_index, pore_porosity_begin);
  if (_use_prescribed_two_population_state &&
      _population_porosity_floors[population_index] > MetaPhysicL::raw_value(floor))
    floor = _population_porosity_floors[population_index];
  return floor;
}

template <bool is_ad>
typename PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::HydrostaticStressState
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::evaluateIndependentHydrostaticStress(
    const GenericReal<is_ad> & matrix_hydro_stress, const PorePorosityState & pore_porosity) const
{
  if (!_use_prescribed_two_population_state)
    return Base::evaluateIndependentHydrostaticStress(matrix_hydro_stress, pore_porosity);

  const auto total_porosity = pore_porosity[0] + pore_porosity[1];
  if (!(total_porosity > 0.0))
    mooseException("In ", this->_name, ": prescribed two-population porosity must be positive.");

  const std::array<Real, Base::MAX_HYDROSTATIC_STRESS_POPULATIONS> reference_porosity = {
      _initial_population_0_fraction * _test_initial_total_porosity,
      (1.0 - _initial_population_0_fraction) * _test_initial_total_porosity};

  auto state = HydrostaticStressState{};
  state.population_count = Base::MAX_HYDROSTATIC_STRESS_POPULATIONS;
  for (auto population_index = 0u;
       population_index < Base::MAX_HYDROSTATIC_STRESS_POPULATIONS;
       ++population_index)
  {
    auto & population = state.populations[population_index];
    population.fraction = pore_porosity[population_index] / total_porosity;

    auto pressure = GenericReal<is_ad>(_population_pressures[population_index]);
    for (auto porosity_index = 0u;
         porosity_index < Base::MAX_HYDROSTATIC_STRESS_POPULATIONS;
         ++porosity_index)
    {
      const auto derivative =
          _population_pressure_derivatives[population_index * Base::MAX_HYDROSTATIC_STRESS_POPULATIONS +
                                           porosity_index];
      pressure += derivative * (pore_porosity[porosity_index] - reference_porosity[porosity_index]);
      population.deffective_hydro_dporosity[porosity_index] = derivative;
    }
    population.effective_hydro_stress = matrix_hydro_stress + pressure;
  }

  state.effective_hydro_stress = pore_porosity[0] > 0.0
                                     ? state.populations[0].effective_hydro_stress
                                     : state.populations[1].effective_hydro_stress;
  return state;
}

template <bool is_ad>
void
PorousViscoplasticityStressUpdateTestStateTempl<is_ad>::independentPorePorosityStateAccepted(
    const GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const PorePorosityState & pore_porosity)
{
  _test_population_0_porosity[this->_qp] = pore_porosity[0];
  _test_population_1_porosity[this->_qp] = pore_porosity[1];

  const auto state = evaluateIndependentHydrostaticStress(this->_hydro_stress, pore_porosity);
  _test_population_0_effective_hydrostatic_stress[this->_qp] =
      state.populations[0].effective_hydro_stress;
  _test_population_1_effective_hydrostatic_stress[this->_qp] =
      state.populations[1].effective_hydro_stress;

  Base::independentPorePorosityStateAccepted(inelastic_strain_increment, pore_porosity);
}

InputParameters
PorousViscoplasticityStressUpdateTest::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription(
      "Test-only porous viscoplastic stress update used to exercise protected generic LPS kernels "
      "and, in later tests, controlled generic constitutive extension points.");
  params.addParam<bool>(
      "run_kernel_checks", false, "Run direct protected-kernel and population-projection checks.");
  params.addParam<MooseEnum>(
      "failure_point",
      MooseEnum("none preparation estimation adaptive_attempt post_accept tangent_replay", "none"),
      "Generic constitutive stage at which to inject std::runtime_error for rollback testing.");
  return params;
}

PorousViscoplasticityStressUpdateTest::PorousViscoplasticityStressUpdateTest(
    const InputParameters & parameters)
  : Base(parameters),
    _run_kernel_checks(getParam<bool>("run_kernel_checks")),
    _failure_point(getParam<MooseEnum>("failure_point"))
{
}

std::string
PorousViscoplasticityStressUpdateTest::failurePointName() const
{
  return _failure_point;
}

void
PorousViscoplasticityStressUpdateTest::throwInjectedFailure(const char * stage) const
{
  throw std::runtime_error(std::string("Injected nonrecoverable porous-LPS test failure at ") +
                           stage);
}

PorousViscoplasticityStressUpdateTest::RollbackState
PorousViscoplasticityStressUpdateTest::captureRollbackState(
    const RankTwoTensor & strain_increment,
    const RankTwoTensor & inelastic_strain_increment,
    const RankTwoTensor & stress,
    const RankFourTensor & tangent) const
{
  auto state = RollbackState{};
  state.strain_increment = strain_increment;
  state.inelastic_strain_increment = inelastic_strain_increment;
  state.stress = stress;
  state.tangent = tangent;

  state.intermediate_porosity = _intermediate_porosity;
  state.effective_inelastic_strain = _effective_inelastic_strain[_qp];
  state.inelastic_strain = _inelastic_strain[_qp];
  state.effective_inelastic_strain_rate = _effective_inelastic_strain_rate[_qp];
  state.substep_control_inelastic_strain_rate = _substep_control_inelastic_strain_rate[_qp];
  state.hydro_stress = _hydro_stress;
  state.gauge_stress = _gauge_stress[_qp];
  state.gauge_stresses.reserve(_gauge_stress_laws.size());
  for (const auto * gauge_stress : _gauge_stress_laws)
    state.gauge_stresses.push_back((*gauge_stress)[_qp]);

  state.population_0_porosity = _test_population_0_porosity[_qp];
  state.population_1_porosity = _test_population_1_porosity[_qp];
  state.constitutive_retries = _performance_counters.constitutive_retries;
  return state;
}

void
PorousViscoplasticityStressUpdateTest::verifyRollbackState(
    const RollbackState & expected,
    const RankTwoTensor & strain_increment,
    const RankTwoTensor & inelastic_strain_increment,
    const RankTwoTensor & stress,
    const RankFourTensor & tangent) const
{
  const auto check = [this](const bool condition, const char * field)
  {
    if (!condition)
      mooseError("In ",
                 _name,
                 ": generic unexpected-exception rollback verification failed at ",
                 failurePointName(),
                 ". Field was not restored: ",
                 field,
                 ".");
  };

  check(rollbackTensorEqual(strain_increment, expected.strain_increment), "strain_increment");
  check(rollbackTensorEqual(inelastic_strain_increment, expected.inelastic_strain_increment),
        "inelastic_strain_increment");
  check(rollbackTensorEqual(stress, expected.stress), "stress");
  check(rollbackTensorEqual(tangent, expected.tangent), "tangent_operator");

  check(rollbackNearlyEqual(_intermediate_porosity, expected.intermediate_porosity),
        "intermediate_porosity");
  check(rollbackNearlyEqual(_effective_inelastic_strain[_qp], expected.effective_inelastic_strain),
        "effective_inelastic_strain");
  check(rollbackTensorEqual(_inelastic_strain[_qp], expected.inelastic_strain),
        "inelastic_strain");
  check(rollbackNearlyEqual(_effective_inelastic_strain_rate[_qp],
                            expected.effective_inelastic_strain_rate),
        "effective_inelastic_strain_rate");
  check(rollbackNearlyEqual(_substep_control_inelastic_strain_rate[_qp],
                            expected.substep_control_inelastic_strain_rate),
        "substep_control_inelastic_strain_rate");
  check(rollbackNearlyEqual(_hydro_stress, expected.hydro_stress), "hydro_stress");
  check(rollbackNearlyEqual(_gauge_stress[_qp], expected.gauge_stress), "gauge_stress");
  check(_gauge_stress_laws.size() == expected.gauge_stresses.size(), "gauge_stress_law_count");
  for (auto law_index = std::size_t{0}; law_index < _gauge_stress_laws.size(); ++law_index)
    check(rollbackNearlyEqual((*_gauge_stress_laws[law_index])[_qp],
                              expected.gauge_stresses[law_index]),
          "gauge_stress_law");
  check(rollbackNearlyEqual(_test_population_0_porosity[_qp], expected.population_0_porosity),
        "population_0_porosity");
  check(rollbackNearlyEqual(_test_population_1_porosity[_qp], expected.population_1_porosity),
        "population_1_porosity");

  check(rollbackNearlyEqual(constitutiveTimeStep(), globalTimeStep()), "constitutive_timestep");
  check(_performance_counters.constitutive_retries == expected.constitutive_retries,
        "constitutive_retries counter");

  if (_failure_point == "preparation" || _failure_point == "estimation")
    check(_accepted_state_calls == 0, "accepted state before pre-attempt injected failure");
  else if (_failure_point == "adaptive_attempt")
    check(_accepted_state_calls == 1,
          "unexpected exception must escape the first local attempt without adaptive retry");
  else if (_failure_point == "post_accept")
  {
    check(_accepted_path_substeps > 1, "post-accept rollback requires a multi-substep path");
    check(_accepted_state_calls == 1,
          "post-accept failure must occur before the second local state is accepted");
  }
  else if (_failure_point == "tangent_replay")
  {
    check(_accepted_path_substeps > 1, "tangent replay requires a multi-substep accepted path");
    check(_accepted_state_calls == _accepted_path_substeps + 1,
          "tangent replay failure must occur on the first accepted replay state");
  }

  Moose::out << "Verified generic unexpected-exception rollback at " << failurePointName()
             << std::endl;
}

void
PorousViscoplasticityStressUpdateTest::updateStateSubstep(
    RankTwoTensor & strain_increment,
    RankTwoTensor & inelastic_strain_increment,
    const RankTwoTensor & rotation_increment,
    RankTwoTensor & stress_new,
    const RankTwoTensor & stress_old,
    const RankFourTensor & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    const bool compute_full_tangent_operator,
    RankFourTensor & tangent_operator)
{
  const auto rollback_state = captureRollbackState(
      strain_increment, inelastic_strain_increment, stress_new, tangent_operator);
  _accepted_state_calls = 0;
  _accepted_path_substeps = 0;
  _inside_update_state_substep = true;

  try
  {
    Base::updateStateSubstep(strain_increment,
                             inelastic_strain_increment,
                             rotation_increment,
                             stress_new,
                             stress_old,
                             elasticity_tensor,
                             elastic_strain_old,
                             compute_full_tangent_operator,
                             tangent_operator);
    _inside_update_state_substep = false;
  }
  catch (const std::exception & error)
  {
    _inside_update_state_substep = false;
    const auto expected_message =
        std::string("Injected nonrecoverable porous-LPS test failure at ") + failurePointName();
    if (error.what() != expected_message)
      mooseError("In ",
                 _name,
                 ": unexpected std::exception reached generic rollback test. Expected '",
                 expected_message,
                 "' but received '",
                 error.what(),
                 "'.");

    verifyRollbackState(
        rollback_state, strain_increment, inelastic_strain_increment, stress_new, tangent_operator);
    throw;
  }
  catch (...)
  {
    _inside_update_state_substep = false;
    mooseError("In ", _name, ": non-standard exception reached generic rollback test.");
  }
}

Real
PorousViscoplasticityStressUpdateTest::scalarPorosityFloor() const
{
  if (_inside_update_state_substep && _failure_point == "preparation")
    throwInjectedFailure("preparation");

  return Base::scalarPorosityFloor();
}

unsigned int
PorousViscoplasticityStressUpdateTest::estimateNumberSubsteps(const RankTwoTensor & stress)
{
  if (_inside_update_state_substep && _failure_point == "estimation")
    throwInjectedFailure("estimation");

  return Base::estimateNumberSubsteps(stress);
}

PorousViscoplasticityStressUpdateTest::PorePorosityState
PorousViscoplasticityStressUpdateTest::independentPorePorosityState(
    const Real & total_porosity) const
{
  if (_inside_update_state_substep && _failure_point == "post_accept" &&
      _accepted_state_calls == 1)
    throwInjectedFailure("post_accept");

  return Base::independentPorePorosityState(total_porosity);
}

void
PorousViscoplasticityStressUpdateTest::independentPorePorosityStateAccepted(
    const RankTwoTensor & inelastic_strain_increment, const PorePorosityState & pore_porosity)
{
  Base::independentPorePorosityStateAccepted(inelastic_strain_increment, pore_porosity);

  if (!_inside_update_state_substep)
    return;

  if (_accepted_state_calls == 0)
  {
    const auto local_dt = constitutiveTimeStep();
    if (local_dt > 0.0)
      _accepted_path_substeps =
          static_cast<unsigned int>(std::lround(globalTimeStep() / local_dt));
  }
  ++_accepted_state_calls;

  if (_failure_point == "adaptive_attempt" && _accepted_state_calls == 1)
    throwInjectedFailure("adaptive_attempt");

  if (_failure_point == "tangent_replay" && _accepted_path_substeps > 1 &&
      _accepted_state_calls == _accepted_path_substeps + 1)
    throwInjectedFailure("tangent_replay");
}

InputParameters
ADPorousViscoplasticityStressUpdateTest::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription(
      "AD test-only porous viscoplastic stress update with prescribed independent pore populations.");
  return params;
}

ADPorousViscoplasticityStressUpdateTest::ADPorousViscoplasticityStressUpdateTest(
    const InputParameters & parameters)
  : Base(parameters)
{
}

template class PorousViscoplasticityStressUpdateTestStateTempl<false>;
template class PorousViscoplasticityStressUpdateTestStateTempl<true>;

void
PorousViscoplasticityStressUpdateTest::checkClose(const char * label,
                                                   const Real actual,
                                                   const Real expected,
                                                   const Real relative_tolerance,
                                                   const Real absolute_tolerance) const
{
  const auto scale = std::max(std::abs(actual), std::abs(expected));
  const auto tolerance = absolute_tolerance + relative_tolerance * scale;
  if (!std::isfinite(actual) || !std::isfinite(expected) || std::abs(actual - expected) > tolerance)
    mooseError("In ",
               _name,
               ": porous-LPS kernel check failed for ",
               label,
               ". actual = ",
               actual,
               ", expected = ",
               expected,
               ", tolerance = ",
               tolerance,
               ".");
}

void
PorousViscoplasticityStressUpdateTest::checkN1Derivatives(
    const Real pressure, const Real deffective_hydro_df) const
{
  constexpr auto beta_squared = 2.25;
  constexpr auto dA_df = 2.0 / 3.0;
  constexpr auto gauge_stress = 2.3e8;
  constexpr auto equiv_stress = 1.7e8;
  constexpr auto porosity = 0.18;

  const auto law = CreepLaw{nullptr, 1.0, 0.0};
  auto hydrostatic_stress = HydrostaticStressState{};
  hydrostatic_stress.effective_hydro_stress = pressure;
  hydrostatic_stress.deffective_hydro_df = deffective_hydro_df;

  const auto derivatives =
      computeLpsDerivatives(gauge_stress, hydrostatic_stress, equiv_stress, porosity, law);

  const auto A = 1.0 + 2.0 * porosity / 3.0;
  const auto q2_over_lambda2 = Utility::pow<2>(equiv_stress / gauge_stress);
  const auto left = A * q2_over_lambda2;
  const auto pressure_ratio_squared = Utility::pow<2>(pressure / gauge_stress);
  const auto Z = 1.0 + beta_squared * pressure_ratio_squared;
  const auto Z_lambda = -2.0 * beta_squared * pressure_ratio_squared / gauge_stress;
  const auto Z_p = 2.0 * beta_squared * pressure / Utility::pow<2>(gauge_stress);
  const auto Z_f = Z_p * deffective_hydro_df;
  const auto Z_lambdalambda =
      6.0 * beta_squared * pressure_ratio_squared / Utility::pow<2>(gauge_stress);
  const auto Z_lambdap = -4.0 * beta_squared * pressure / Utility::pow<3>(gauge_stress);
  const auto Z_lambdaf = Z_lambdap * deffective_hydro_df;
  const auto Z_pp = 2.0 * beta_squared / Utility::pow<2>(gauge_stress);
  const auto Z_pf = Z_pp * deffective_hydro_df;

  checkClose("n1 F_lambda", derivatives.F_lambda, -2.0 * left / gauge_stress + porosity * Z_lambda);
  checkClose("n1 F_p", derivatives.F_p, porosity * Z_p);
  checkClose("n1 F_q", derivatives.F_q, 2.0 * A * equiv_stress / Utility::pow<2>(gauge_stress));
  checkClose("n1 F_f", derivatives.F_f, dA_df * q2_over_lambda2 + Z + porosity * Z_f);
  checkClose("n1 F_lambdalambda",
             derivatives.F_lambdalambda,
             6.0 * left / Utility::pow<2>(gauge_stress) + porosity * Z_lambdalambda);
  checkClose("n1 F_lambdap", derivatives.F_lambdap, porosity * Z_lambdap);
  checkClose("n1 F_lambdaq",
             derivatives.F_lambdaq,
             -4.0 * A * equiv_stress / Utility::pow<3>(gauge_stress));
  checkClose("n1 F_lambdaf",
             derivatives.F_lambdaf,
             -2.0 * dA_df * Utility::pow<2>(equiv_stress) / Utility::pow<3>(gauge_stress) +
                 Z_lambda + porosity * Z_lambdaf);
  checkClose("n1 F_pp", derivatives.F_pp, porosity * Z_pp);
  checkClose("n1 F_pf", derivatives.F_pf, Z_p + porosity * Z_pf);

  if (pressure == 0.0)
  {
    // At exactly zero signed pressure the n=1 first pressure derivative vanishes, but the finite
    // pressure curvature and the p-f mixed derivative through the pressure closure must survive.
    checkClose("n1 exact-zero F_p", derivatives.F_p, 0.0);
    checkClose("n1 exact-zero F_lambdap", derivatives.F_lambdap, 0.0);
    checkClose("n1 exact-zero F_pp",
               derivatives.F_pp,
               porosity * 2.0 * beta_squared / Utility::pow<2>(gauge_stress));
    checkClose("n1 exact-zero F_pf",
               derivatives.F_pf,
               porosity * 2.0 * beta_squared * deffective_hydro_df / Utility::pow<2>(gauge_stress));

    // Independent central finite differences provide a numerical oracle for the two derivatives
    // most easily lost by an |p|-based exact-zero branch.
    constexpr auto pressure_step = 2.0e4;
    constexpr auto porosity_step = 1.0e-5;
    const auto residual = [=](const Real matrix_pressure, const Real local_porosity)
    {
      const auto effective_pressure =
          matrix_pressure + deffective_hydro_df * (local_porosity - porosity);
      const auto local_A = 1.0 + 2.0 * local_porosity / 3.0;
      return local_A * Utility::pow<2>(equiv_stress / gauge_stress) +
             local_porosity *
                 (1.0 + beta_squared * Utility::pow<2>(effective_pressure / gauge_stress)) -
             1.0;
    };

    const auto F0 = residual(0.0, porosity);
    const auto Fpp_fd =
        (residual(pressure_step, porosity) - 2.0 * F0 + residual(-pressure_step, porosity)) /
        Utility::pow<2>(pressure_step);
    const auto Fpf_fd = (residual(pressure_step, porosity + porosity_step) -
                         residual(pressure_step, porosity - porosity_step) -
                         residual(-pressure_step, porosity + porosity_step) +
                         residual(-pressure_step, porosity - porosity_step)) /
                        (4.0 * pressure_step * porosity_step);

    checkClose("n1 exact-zero F_pp finite difference", derivatives.F_pp, Fpp_fd, 2.0e-7, 1.0e-25);
    checkClose("n1 exact-zero F_pf finite difference", derivatives.F_pf, Fpf_fd, 2.0e-7, 1.0e-18);
  }
}

void
PorousViscoplasticityStressUpdateTest::checkHigherPowerNearZeroPressure(
    const Real power) const
{
  if (!(power > 1.0))
    mooseError("In ", _name, ": higher-power near-zero check requires n > 1.");

  constexpr auto beta = 1.5;
  constexpr auto gauge_stress = 2.3e8;
  constexpr auto equiv_stress = 1.7e8;
  constexpr auto porosity = 0.18;
  constexpr auto deffective_hydro_df = 6.0e7;

  const auto alpha = (power - 1.0) / (power + 1.0);
  const auto exponent = (power + 1.0) / power;
  const auto law = CreepLaw{nullptr, power, alpha};

  const auto derivatives_at = [&](const Real pressure)
  {
    auto state = HydrostaticStressState{};
    state.effective_hydro_stress = pressure;
    state.deffective_hydro_df = deffective_hydro_df;
    return computeLpsDerivatives(gauge_stress, state, equiv_stress, porosity, law);
  };

  const auto zero = derivatives_at(0.0);
  const auto h_zero = computeHDerivatives(power, 0.0);
  checkClose("higher-power H(0)", h_zero.value, 1.0);
  checkClose("higher-power H'(0)", h_zero.first, 0.0);
  checkClose("higher-power exact-zero H'' convention", h_zero.second, 0.0);
  checkClose("higher-power exact-zero F_p convention", zero.F_p, 0.0);
  checkClose("higher-power exact-zero F_pp convention", zero.F_pp, 0.0);
  checkClose("higher-power exact-zero F_pf convention", zero.F_pf, 0.0);

  const auto beta_to_exponent = std::pow(beta, exponent);
  for (const auto M : std::array<Real, 2>{1.0e-6, 1.0e-8})
  {
    const auto pressure = gauge_stress * M;
    const auto plus = derivatives_at(pressure);
    const auto minus = derivatives_at(-pressure);

    // For n > 1 and M -> 0+, Z = H + alpha/H has
    //   Z_M  ~ (2/n) beta^(1+1/n) M^(1/n),
    //   Z_MM ~ (2/n^2) beta^(1+1/n) M^(1/n-1).
    // Thus the signed first pressure derivative vanishes continuously and changes sign, while
    // the pressure curvature is positive, even in pressure, and diverges toward zero pressure.
    const auto Z_M_asymptotic = 2.0 / power * beta_to_exponent * std::pow(M, 1.0 / power);
    const auto Z_MM_asymptotic =
        2.0 / Utility::pow<2>(power) * beta_to_exponent * std::pow(M, 1.0 / power - 1.0);
    const auto F_p_asymptotic = porosity * Z_M_asymptotic / gauge_stress;
    const auto F_pp_asymptotic = porosity * Z_MM_asymptotic / Utility::pow<2>(gauge_stress);
    const auto F_pf_even_asymptotic = deffective_hydro_df * F_pp_asymptotic;
    const auto F_pf_odd_asymptotic = Z_M_asymptotic / gauge_stress;

    checkClose("higher-power positive near-zero F_p", plus.F_p, F_p_asymptotic, 1.0e-5, 5.0e-30);
    checkClose("higher-power negative near-zero F_p", minus.F_p, -F_p_asymptotic, 1.0e-5, 5.0e-30);
    checkClose("higher-power positive near-zero F_pp", plus.F_pp, F_pp_asymptotic, 1.0e-5, 5.0e-30);
    checkClose(
        "higher-power negative near-zero F_pp", minus.F_pp, F_pp_asymptotic, 1.0e-5, 5.0e-30);
    checkClose("higher-power positive near-zero F_pf",
               plus.F_pf,
               F_pf_even_asymptotic + F_pf_odd_asymptotic,
               1.0e-5,
               5.0e-30);
    checkClose("higher-power negative near-zero F_pf",
               minus.F_pf,
               F_pf_even_asymptotic - F_pf_odd_asymptotic,
               1.0e-5,
               5.0e-30);
  }

  // A centered pressure crossing does not approach a finite second derivative. Its secant
  // curvature grows as h^(-(n-1)/n), which explicitly distinguishes this C1/non-C2 point from
  // the finite-curvature n = 1 case.
  const auto residual = [&](const Real pressure)
  {
    const auto M = std::abs(pressure) / gauge_stress;
    const auto mod = std::pow(beta * M, exponent);
    const auto H = std::pow(1.0 + mod / power, power);
    const auto Z = H + alpha / H;
    const auto A = 1.0 + 2.0 * porosity / 3.0;
    return A * Utility::pow<2>(equiv_stress / gauge_stress) + porosity * Z - 1.0 -
           alpha * Utility::pow<2>(porosity);
  };

  const auto crossing_curvature = [&](const Real M)
  {
    const auto pressure = gauge_stress * M;
    return (residual(pressure) - 2.0 * residual(0.0) + residual(-pressure)) /
           Utility::pow<2>(pressure);
  };

  constexpr auto M_large = 1.0e-5;
  constexpr auto M_small = 1.0e-7;
  const auto curvature_large = crossing_curvature(M_large);
  const auto curvature_small = crossing_curvature(M_small);
  if (!(curvature_large > 0.0 && curvature_small > curvature_large))
    mooseError("In ",
               _name,
               ": higher-power zero-crossing curvature did not grow toward p_eff = 0 for n = ",
               power,
               ".");

  const auto measured_ratio = curvature_small / curvature_large;
  const auto expected_ratio = std::pow(M_small / M_large, -(power - 1.0) / power);
  checkClose("higher-power zero-crossing curvature scaling",
             measured_ratio,
             expected_ratio,
             1.0e-4,
             1.0e-12);

  // Repeat the signed/zero contract population-by-population for the independent-pore kernel.
  const auto pore_porosity = PorePorosityState{0.08, 0.13};
  const auto independent_at = [&](const Real pressure)
  {
    auto state = HydrostaticStressState{};
    state.population_count = 2;
    state.populations[0].fraction = 0.4;
    state.populations[0].effective_hydro_stress = pressure;
    state.populations[0].deffective_hydro_dporosity = {deffective_hydro_df, 0.0};
    state.populations[1].fraction = 0.6;
    state.populations[1].effective_hydro_stress = -8.0e7;
    return computeIndependentLpsDerivatives(gauge_stress, state, equiv_stress, pore_porosity, law);
  };

  const auto independent_zero = independent_at(0.0);
  checkClose("independent higher-power exact-zero population F_p convention",
             independent_zero.population_F_p[0],
             0.0);
  checkClose("independent higher-power exact-zero population F_pp convention",
             independent_zero.population_F_pp[0],
             0.0);
  checkClose("independent higher-power exact-zero mixed convention",
             independent_zero.population_F_pporosity[0][0],
             0.0);

  constexpr auto independent_M = 1.0e-7;
  const auto independent_pressure = gauge_stress * independent_M;
  const auto independent_plus = independent_at(independent_pressure);
  const auto independent_minus = independent_at(-independent_pressure);
  const auto Z_M_asymptotic = 2.0 / power * beta_to_exponent * std::pow(independent_M, 1.0 / power);
  const auto Z_MM_asymptotic =
      2.0 / Utility::pow<2>(power) * beta_to_exponent * std::pow(independent_M, 1.0 / power - 1.0);
  const auto population_F_p_asymptotic = pore_porosity[0] * Z_M_asymptotic / gauge_stress;
  const auto population_F_pp_asymptotic =
      pore_porosity[0] * Z_MM_asymptotic / Utility::pow<2>(gauge_stress);
  const auto population_mixed_even = deffective_hydro_df * population_F_pp_asymptotic;
  const auto population_mixed_odd = Z_M_asymptotic / gauge_stress;

  checkClose("independent higher-power positive population F_p",
             independent_plus.population_F_p[0],
             population_F_p_asymptotic,
             1.0e-5,
             5.0e-30);
  checkClose("independent higher-power negative population F_p",
             independent_minus.population_F_p[0],
             -population_F_p_asymptotic,
             1.0e-5,
             5.0e-30);
  checkClose("independent higher-power positive population F_pp",
             independent_plus.population_F_pp[0],
             population_F_pp_asymptotic,
             1.0e-5,
             5.0e-30);
  checkClose("independent higher-power negative population F_pp",
             independent_minus.population_F_pp[0],
             population_F_pp_asymptotic,
             1.0e-5,
             5.0e-30);
  checkClose("independent higher-power positive mixed derivative",
             independent_plus.population_F_pporosity[0][0],
             population_mixed_even + population_mixed_odd,
             1.0e-5,
             5.0e-30);
  checkClose("independent higher-power negative mixed derivative",
             independent_minus.population_F_pporosity[0][0],
             population_mixed_even - population_mixed_odd,
             1.0e-5,
             5.0e-30);
}

void
PorousViscoplasticityStressUpdateTest::checkHAndCreepKernels() const
{
  const std::array<Real, 3> powers = {1.0, 3.0, 4.5};
  const std::array<Real, 5> M_values = {0.0, 1.0e-12, 0.03, 0.7, 5.0};

  for (const auto power : powers)
    for (const auto M : M_values)
    {
      const auto raw = computeHValueFirstRaw(power, M);
      const auto generic = computeHValueFirst(power, M);
      const auto full = computeHDerivatives(power, M);
      checkClose("raw/generic H", raw.value, generic.value);
      checkClose("raw/generic dH/dM", raw.first, generic.first);
      checkClose("raw/full H", raw.value, full.value);
      checkClose("raw/full dH/dM", raw.first, full.first);
    }

  const auto n1_zero = computeHDerivatives(1.0, 0.0);
  checkClose("n1 H(0)", n1_zero.value, 1.0);
  checkClose("n1 H'(0)", n1_zero.first, 0.0);
  checkClose("n1 H''(0)", n1_zero.second, 4.5);

  constexpr auto coefficient = 2.7e-20;
  constexpr auto gauge_stress = 1.3e4;
  for (const auto power : powers)
  {
    const auto law = CreepLaw{nullptr, power, (power - 1.0) / (power + 1.0)};
    const auto optimized = computeCreepRate(law, coefficient, gauge_stress);
    const auto reference = coefficient * std::pow(gauge_stress, power);
    checkClose("optimized creep-rate kernel", optimized, reference);
  }
}

void
PorousViscoplasticityStressUpdateTest::checkGaugeResidualKernels()
{
  constexpr auto equiv_stress = 1.3e8;
  constexpr auto gauge_stress = 2.4e8;
  constexpr auto porosity = 0.21;

  auto hydrostatic_stress = HydrostaticStressState{};
  hydrostatic_stress.population_count = 2;
  hydrostatic_stress.populations[0].fraction = 0.35;
  hydrostatic_stress.populations[0].effective_hydro_stress = -4.0e7;
  hydrostatic_stress.populations[1].fraction = 0.65;
  hydrostatic_stress.populations[1].effective_hydro_stress = 1.1e8;

  for (const auto power : std::array<Real, 3>{1.0, 3.0, 4.5})
  {
    const auto law = CreepLaw{nullptr, power, (power - 1.0) / (power + 1.0)};
    auto raw_derivative = 0.0;
    const auto raw = computeGaugeResidualRaw(
        equiv_stress, gauge_stress, hydrostatic_stress, porosity, law, raw_derivative);
    auto generic_derivative = Real(0.0);
    const auto generic = computeGaugeResidual(
        equiv_stress, gauge_stress, hydrostatic_stress, porosity, law, generic_derivative);

    checkClose("raw/generic gauge residual", raw, generic);
    checkClose("raw/generic gauge derivative", raw_derivative, generic_derivative);
  }

  const auto law = CreepLaw{nullptr, 3.0, 0.5};
  const auto saved_n3 = _gauge_solve_state.n3;
  auto & n3 = _gauge_solve_state.n3;
  n3 = {};
  n3.deviatoric_prefactor = (1.0 + porosity / 1.5) * Utility::pow<2>(equiv_stress);
  n3.residual_constant = -1.0 - law.power_factor * Utility::pow<2>(porosity);
  n3.power_factor = law.power_factor;

  using std::abs;
  using std::pow;
  constexpr auto beta = 1.5;
  constexpr auto four_thirds = 4.0 / 3.0;
  for (auto population_index = 0u; population_index < 2; ++population_index)
  {
    const auto & population = hydrostatic_stress.populations[population_index];
    const auto active_index = n3.active_population_count++;
    n3.porosity_weights[active_index] = porosity * population.fraction;
    n3.pressure_four_thirds[active_index] =
        pow(beta * abs(population.effective_hydro_stress), four_thirds);
  }

  auto optimized_derivative = 0.0;
  const auto optimized = computeGaugeResidualN3Raw(gauge_stress, optimized_derivative);
  auto reference_derivative = 0.0;
  const auto reference = computeGaugeResidualRaw(
      equiv_stress, gauge_stress, hydrostatic_stress, porosity, law, reference_derivative);
  _gauge_solve_state.n3 = saved_n3;

  checkClose("specialized/general n3 gauge residual", optimized, reference, 2.0e-13, 5.0e-15);
  checkClose("specialized/general n3 gauge derivative",
             optimized_derivative,
             reference_derivative,
             2.0e-13,
             5.0e-30);
}

void
PorousViscoplasticityStressUpdateTest::checkIndependentDerivativeKernels() const
{
  constexpr auto gauge_stress = 2.1e8;
  constexpr auto equiv_stress = 1.2e8;
  const auto pore_porosity = PorePorosityState{0.08, 0.13};

  auto hydrostatic_stress = HydrostaticStressState{};
  hydrostatic_stress.population_count = 2;
  hydrostatic_stress.populations[0].fraction = 0.4;
  hydrostatic_stress.populations[0].effective_hydro_stress = 0.0;
  hydrostatic_stress.populations[0].deffective_hydro_dporosity = {3.0e7, -1.0e7};
  hydrostatic_stress.populations[1].fraction = 0.6;
  hydrostatic_stress.populations[1].effective_hydro_stress = -8.0e7;
  hydrostatic_stress.populations[1].deffective_hydro_dporosity = {-2.0e7, 4.0e7};

  for (const auto power : std::array<Real, 3>{1.0, 3.0, 4.5})
  {
    const auto law = CreepLaw{nullptr, power, (power - 1.0) / (power + 1.0)};
    const auto flow = computeIndependentLpsFlowDerivatives(
        gauge_stress, hydrostatic_stress, equiv_stress, pore_porosity, law);
    const auto full = computeIndependentLpsDerivatives(
        gauge_stress, hydrostatic_stress, equiv_stress, pore_porosity, law);

    checkClose("independent flow/full F_lambda", flow.F_lambda, full.F_lambda);
    checkClose("independent flow/full F_p", flow.F_p, full.F_p);
    for (auto population_index = 0u; population_index < 2; ++population_index)
      checkClose("independent flow/full population F_p",
                 flow.population_F_p[population_index],
                 full.population_F_p[population_index]);

    if (power == 1.0)
    {
      constexpr auto beta_squared = 2.25;
      const auto expected_zero_pressure_curvature =
          pore_porosity[0] * 2.0 * beta_squared / Utility::pow<2>(gauge_stress);
      checkClose("independent n1 exact-zero population F_p", full.population_F_p[0], 0.0);
      checkClose(
          "independent n1 exact-zero population F_lambdap", full.population_F_lambdap[0], 0.0);
      checkClose("independent n1 exact-zero population F_pp",
                 full.population_F_pp[0],
                 expected_zero_pressure_curvature);
      checkClose("independent n1 exact-zero mixed F_pporosity",
                 full.population_F_pporosity[0][0],
                 expected_zero_pressure_curvature *
                     hydrostatic_stress.populations[0].deffective_hydro_dporosity[0]);
    }
  }
}

void
PorousViscoplasticityStressUpdateTest::checkIndependentProjectionKernels() const
{
  const auto unconstrained = PorePorosityState{0.02, -0.01};
  const auto pore_porosity_begin = PorePorosityState{0.08, 0.12};
  const auto dilution_coefficient = PorePorosityState{0.2, 0.3};
  constexpr auto solid_fraction_old = 0.8;

  const auto no_floor = projectIndependentPopulationVolumetricIncrement(
      unconstrained, pore_porosity_begin, dilution_coefficient, solid_fraction_old, {false, false});
  checkClose("independent projection no-floor population 0",
             no_floor.population_volumetric_increment[0],
             unconstrained[0]);
  checkClose("independent projection no-floor population 1",
             no_floor.population_volumetric_increment[1],
             unconstrained[1]);
  if (no_floor.active_count != 0)
    mooseError("In ", _name, ": no-floor projection reported an active floor.");

  // The dedicated input sets minimum_porosity = 0.05. With initial population porosities 0.08 and
  // 0.12, the proportional generic floors are therefore 0.02 and 0.03.
  constexpr auto floor_0 = 0.02;
  constexpr auto floor_1 = 0.03;
  const auto one_floor_0 = projectIndependentPopulationVolumetricIncrement(
      unconstrained, pore_porosity_begin, dilution_coefficient, solid_fraction_old, {true, false});
  const auto expected_one_floor_0 =
      (floor_0 - pore_porosity_begin[0] + dilution_coefficient[0] * unconstrained[1]) /
      (1.0 - dilution_coefficient[0]);
  checkClose("independent projection one-floor population 0",
             one_floor_0.population_volumetric_increment[0],
             expected_one_floor_0);
  checkClose("independent projection one-floor free population 1",
             one_floor_0.population_volumetric_increment[1],
             unconstrained[1]);
  if (one_floor_0.active_count != 1 || one_floor_0.active_population != 0)
    mooseError("In ", _name, ": population-0 floor projection reported the wrong active set.");

  const auto one_floor_1 = projectIndependentPopulationVolumetricIncrement(
      unconstrained, pore_porosity_begin, dilution_coefficient, solid_fraction_old, {false, true});
  const auto expected_one_floor_1 =
      (floor_1 - pore_porosity_begin[1] + dilution_coefficient[1] * unconstrained[0]) /
      (1.0 - dilution_coefficient[1]);
  checkClose("independent projection free population 0",
             one_floor_1.population_volumetric_increment[0],
             unconstrained[0]);
  checkClose("independent projection one-floor population 1",
             one_floor_1.population_volumetric_increment[1],
             expected_one_floor_1);
  if (one_floor_1.active_count != 1 || one_floor_1.active_population != 1)
    mooseError("In ", _name, ": population-1 floor projection reported the wrong active set.");

  const auto two_floor = projectIndependentPopulationVolumetricIncrement(
      unconstrained, pore_porosity_begin, dilution_coefficient, solid_fraction_old, {true, true});
  const auto constrained_total_increment =
      (floor_0 + floor_1 - pore_porosity_begin[0] - pore_porosity_begin[1]) / solid_fraction_old;
  checkClose("independent projection two-floor population 0",
             two_floor.population_volumetric_increment[0],
             floor_0 - pore_porosity_begin[0] +
                 dilution_coefficient[0] * constrained_total_increment);
  checkClose("independent projection two-floor population 1",
             two_floor.population_volumetric_increment[1],
             floor_1 - pore_porosity_begin[1] +
                 dilution_coefficient[1] * constrained_total_increment);
  if (two_floor.active_count != 2)
    mooseError("In ", _name, ": two-floor projection did not report both active floors.");
}

void
PorousViscoplasticityStressUpdateTest::runKernelChecks()
{
  checkN1Derivatives(0.0, 6.0e7);
  checkN1Derivatives(6.2e7, 0.0);
  checkN1Derivatives(-6.2e7, 0.0);
  checkHigherPowerNearZeroPressure(3.0);
  checkHigherPowerNearZeroPressure(4.5);
  checkHAndCreepKernels();
  checkGaugeResidualKernels();
  checkIndependentDerivativeKernels();
  checkIndependentProjectionKernels();
}

void
PorousViscoplasticityStressUpdateTest::initQpStatefulProperties()
{
  Base::initQpStatefulProperties();

  if (!_run_kernel_checks || _kernel_checks_complete)
    return;

  runKernelChecks();
  _kernel_checks_complete = true;
  Moose::out << "Porous LPS implementation kernel checks passed" << std::endl;
}
