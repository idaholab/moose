//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFrictionalForceLMMechanicalContact.h"
#include "DisplacedProblem.h"
#include "Assembly.h"
#include "MortarContactUtils.h"
#include "ContactFrictionUtils.h"
#include "LMWeightedVelocitiesUserObject.h"
#include "WeightedVelocitiesUserObject.h"

#include "metaphysicl/metaphysicl_version.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#if METAPHYSICL_MAJOR_VERSION < 2
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#else
#include "metaphysicl/parallel_dynamic_array_wrapper.h"
#endif
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

#include <cmath>

registerMooseObject("ContactApp", ComputeFrictionalForceLMMechanicalContact);

InputParameters
ComputeFrictionalForceLMMechanicalContact::validParams()
{
  InputParameters params = ComputeWeightedGapLMMechanicalContact::validParams();
  params.addClassDescription("Computes the tangential frictional forces");
  params.addRequiredCoupledVar("friction_lm", "The frictional Lagrange's multiplier");
  params.addCoupledVar("friction_lm_dir",
                       "The frictional Lagrange's multiplier for an addtional direction.");
  params.addParam<FunctionName>(
      "function_friction",
      "Coupled function to evaluate friction with values from contact pressure and relative "
      "tangential velocities");
  params.addParam<Real>("c_t", 1e0, "Numerical parameter for tangential constraints");
  params.addParam<Real>(
      "epsilon",
      1.0e-7,
      "Minimum value of contact pressure that will trigger frictional enforcement");
  params.addRangeCheckedParam<Real>(
      "mu", "mu > 0", "The friction coefficient for the Coulomb friction law");
  params.addParam<MooseEnum>("friction_coefficient_regularization",
                             Moose::Contact::frictionCoefficientRegularizationOptions(),
                             "The regularization applied to the Coulomb friction coefficient.");
  params.addRangeCheckedParam<Real>("friction_reference_slip",
                                    0.0,
                                    "friction_reference_slip >= 0",
                                    "Positive slip-increment scale used by ARCTAN_SLIP.");
  params.addRangeCheckedParam<Real>(
      "friction_elastic_slip",
      0.0,
      "friction_elastic_slip >= 0",
      "Maximum reversible tangential slip before reaching the Coulomb capacity.");
  params.addRequiredParam<UserObjectName>("weighted_velocities_uo",
                                          "The weighted tangential velocities user object.");

  return params;
}

ComputeFrictionalForceLMMechanicalContact::ComputeFrictionalForceLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeWeightedGapLMMechanicalContact(parameters),
    _weighted_velocities_uo(getUserObject<WeightedVelocitiesUserObject>("weighted_velocities_uo")),
    _lm_weighted_velocities_uo(nullptr),
    _c_t(getParam<Real>("c_t")),
    _secondary_x_dot(_secondary_var.adUDot()),
    _primary_x_dot(_primary_var.adUDotNeighbor()),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _epsilon(getParam<Real>("epsilon")),
    _mu(isParamValid("function_friction") ? std::numeric_limits<double>::quiet_NaN()
                                          : getParam<Real>("mu")),
    _friction_coefficient_regularization(
        getParam<MooseEnum>("friction_coefficient_regularization")
            .getEnum<Moose::Contact::FrictionCoefficientRegularization>()),
    _friction_reference_slip(getParam<Real>("friction_reference_slip")),
    _friction_elastic_slip(getParam<Real>("friction_elastic_slip")),
    _committed_elastic_slip_state(
        declareRestartableData<ElasticSlipStateMap>("committed_elastic_slip_state")),
    _candidate_elastic_slip_state(
        declareRestartableData<ElasticSlipStateMap>("candidate_elastic_slip_state")),
    _candidate_elastic_slip_step(declareRestartableData<int>("candidate_elastic_slip_step", -1)),
    _function_friction(isParamValid("function_friction") ? &getFunction("function_friction")
                                                         : nullptr),
    _has_friction_function(isParamValid("function_friction")),
    _3d(_has_disp_z)

{
  if (parameters.isParamSetByUser("mu") && _has_friction_function)
    paramError(
        "mu",
        "Please only provide friction either as a function or as a constant value, but not both.");
  else if (!parameters.isParamSetByUser("mu") && !_has_friction_function)
    paramError("mu", "Please provide a value or a function for the coefficient of friction.");

  if (!getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "'use_displaced_mesh' must be true for the "
               "ComputeFrictionalForceLMMechanicalContact object");

  if (_3d && !isParamValid("friction_lm_dir"))
    paramError("friction_lm_dir",
               "Three-dimensional mortar frictional contact simulations require an additional "
               "frictional Lagrange's multiplier to enforce a second tangential pressure");

  if (_friction_coefficient_regularization !=
          Moose::Contact::FrictionCoefficientRegularization::NONE &&
      _friction_reference_slip <= 0.0)
    paramError("friction_reference_slip",
               "A positive friction_reference_slip is required when "
               "friction_coefficient_regularization is not NONE.");

  if (_friction_coefficient_regularization !=
          Moose::Contact::FrictionCoefficientRegularization::NONE &&
      _friction_elastic_slip > 0.0)
    paramError("friction_coefficient_regularization",
               "friction_coefficient_regularization and friction_elastic_slip are mutually "
               "exclusive.");

  _friction_vars.push_back(getVar("friction_lm", 0));

  if (_3d)
    _friction_vars.push_back(getVar("friction_lm_dir", 0));

  if (!_friction_vars[0]->isNodal())
    if (_friction_vars[0]->feType().order != static_cast<Order>(0))
      paramError(
          "friction_lm",
          "Frictional contact constraints only support elemental variables of CONSTANT order");

  const bool uses_friction_regularization =
      _friction_elastic_slip > 0.0 || _friction_coefficient_regularization !=
                                          Moose::Contact::FrictionCoefficientRegularization::NONE;
  if (uses_friction_regularization && !_var->isNodal())
    paramError(_friction_elastic_slip > 0.0 ? "friction_elastic_slip"
                                            : "friction_coefficient_regularization",
               "Friction regularization currently requires a nodal normal mortar Lagrange "
               "multiplier.");
  if (uses_friction_regularization)
  {
    _lm_weighted_velocities_uo =
        dynamic_cast<const LMWeightedVelocitiesUserObject *>(&_weighted_velocities_uo);
    if (!_lm_weighted_velocities_uo)
      paramError("weighted_velocities_uo",
                 "Friction regularization requires an LMWeightedVelocitiesUserObject.");

    if (_var->feType().order != FIRST)
      paramError(_friction_elastic_slip > 0.0 ? "friction_elastic_slip"
                                              : "friction_coefficient_regularization",
                 "Friction regularization currently requires first-order nodal mortar Lagrange "
                 "multipliers because its physical slip increment is normalized by a lumped "
                 "mortar weight.");
    const_cast<LMWeightedVelocitiesUserObject &>(*_lm_weighted_velocities_uo)
        .requestFrictionRegularizationData(_friction_elastic_slip > 0.0);
    for (const auto i : index_range(_friction_vars))
    {
      if (_friction_vars[i]->feType() != _var->feType())
        paramError(i == 0 ? "friction_lm" : "friction_lm_dir",
                   "Friction regularization requires the normal and tangential mortar Lagrange "
                   "multipliers to use the same first-order nodal finite element type.");
      if (_friction_vars[i]->activeSubdomains() != _var->activeSubdomains())
        paramError(i == 0 ? "friction_lm" : "friction_lm_dir",
                   "Friction regularization requires the normal and tangential mortar Lagrange "
                   "multipliers to be defined on the same subdomains.");
    }
  }
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpProperties()
{
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpIProperties()
{
}

void
ComputeFrictionalForceLMMechanicalContact::residualSetup()
{
  if (_friction_elastic_slip > 0.0)
  {
    _candidate_elastic_slip_state.clear();
    _candidate_elastic_slip_step = _t_step;
  }
}

void
ComputeFrictionalForceLMMechanicalContact::timestepSetup()
{
  ComputeWeightedGapLMMechanicalContact::timestepSetup();

  if (_friction_elastic_slip <= 0.0)
    return;

  if (_candidate_elastic_slip_step == _t_step - 1)
    _committed_elastic_slip_state = _candidate_elastic_slip_state;

  _candidate_elastic_slip_state.clear();
  _candidate_elastic_slip_step = -1;
}

void
ComputeFrictionalForceLMMechanicalContact::meshChanged()
{
  if (_friction_elastic_slip > 0.0 &&
      (!_committed_elastic_slip_state.empty() || !_candidate_elastic_slip_state.empty()))
    mooseError("friction_elastic_slip does not support mesh topology changes after elastic-slip "
               "history has been staged or committed.");
}

void
ComputeFrictionalForceLMMechanicalContact::post()
{
  const auto & dof_to_weighted_tangential_velocity =
      _weighted_velocities_uo.dofToWeightedVelocities();

  const std::unordered_map<const DofObject *, std::pair<ADReal, Real>> & dof_to_weighted_gap =
      _weighted_gap_uo.dofToWeightedGap();

  // Enforce frictional constraints

  for (const auto & [dof_object, weighted_velocities_pr] : dof_to_weighted_tangential_velocity)
  {
    if (dof_object->processor_id() != this->processor_id())
      continue;

    const auto & [weighted_gap_pr, normalization] =
        libmesh_map_find(dof_to_weighted_gap, dof_object);
    _weighted_gap_ptr = &weighted_gap_pr;
    _normalization_ptr = &normalization;
    _tangential_vel_ptr[0] = &(weighted_velocities_pr[0]);

    if (_3d)
    {
      _tangential_vel_ptr[1] = &(weighted_velocities_pr[1]);
      enforceConstraintOnDof3d(dof_object);
    }
    else
      enforceConstraintOnDof(dof_object);
  }
}

void
ComputeFrictionalForceLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  const auto & dof_to_weighted_tangential_velocity =
      _weighted_velocities_uo.dofToWeightedVelocities();
  const auto & dof_to_weighted_gap = _weighted_gap_uo.dofToWeightedGap();
  // Enforce frictional complementarity constraints
  for (const auto & [dof_object, weighted_velocities_pr] : dof_to_weighted_tangential_velocity)
  {
    // If node inactive, skip
    if ((inactive_lm_nodes.find(static_cast<const Node *>(dof_object)) !=
         inactive_lm_nodes.end()) ||
        (dof_object->processor_id() != this->processor_id()))
      continue;

    _weighted_gap_ptr = &dof_to_weighted_gap.at(dof_object).first;
    _normalization_ptr = &dof_to_weighted_gap.at(dof_object).second;
    _tangential_vel_ptr[0] = &weighted_velocities_pr[0];

    if (_3d)
    {
      _tangential_vel_ptr[1] = &weighted_velocities_pr[1];
      enforceConstraintOnDof3d(dof_object);
    }
    else
      enforceConstraintOnDof(dof_object);
  }
}

void
ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnDof3d(const DofObject * const dof)
{
  using std::max, std::sqrt;

  ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(dof);

  // Get normal LM
  const auto normal_dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  const ADReal & weighted_gap = *_weighted_gap_ptr;
  ADReal contact_pressure = (*_sys.currentSolution())(normal_dof_index);
  Moose::derivInsert(contact_pressure.derivatives(), normal_dof_index, 1.);
  // Get friction LMs
  std::array<const ADReal *, 2> & tangential_vel = _tangential_vel_ptr;
  std::array<dof_id_type, 2> friction_dof_indices;
  std::array<ADReal, 2> friction_lm_values;

  const unsigned int num_tangents = 2;
  for (const auto i : make_range(num_tangents))
  {
    friction_dof_indices[i] = dof->dof_number(_sys.number(), _friction_vars[i]->number(), 0);
    friction_lm_values[i] = (*_sys.currentSolution())(friction_dof_indices[i]);
    Moose::derivInsert(friction_lm_values[i].derivatives(), friction_dof_indices[i], 1.);
  }

  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  const bool regularized =
      _friction_elastic_slip > 0.0 || _friction_coefficient_regularization !=
                                          Moose::Contact::FrictionCoefficientRegularization::NONE;
  ADReal base_mu;
  if (regularized && _has_friction_function)
  {
    const ADReal slip_rate =
        _dt > 0.0 ? _lm_weighted_velocities_uo->tangentialSlipIncrement(dof) / _dt : 0.0;
    base_mu = computeFrictionValue(contact_pressure, slip_rate, 0.0);
  }
  else
    base_mu = computeFrictionValue(contact_pressure,
                                   _dof_to_real_tangential_velocity[dof][0],
                                   _dof_to_real_tangential_velocity[dof][1]);

  ADReal dof_residual;
  ADReal dof_residual_dir;

  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon)
  {
    dof_residual = friction_lm_values[0];
    dof_residual_dir = friction_lm_values[1];
  }
  else
  {
    // Epsilon to avoid automatic differentiation singularity
    const Real epsilon_sqrt = 1.0e-48;

    const auto lamdba_plus_cg = contact_pressure + c * weighted_gap;
    if (_friction_elastic_slip > 0.0)
    {
      const auto elastic_residual =
          computeElasticSlipResidual(dof, friction_lm_values, base_mu, contact_pressure, 2);
      dof_residual = elastic_residual[0];
      dof_residual_dir = elastic_residual[1];
    }
    else if (_friction_coefficient_regularization ==
             Moose::Contact::FrictionCoefficientRegularization::ARCTAN_SLIP)
    {
      const ADReal mu_ad = Moose::Contact::arctanFrictionCoefficient(
          base_mu,
          _lm_weighted_velocities_uo->tangentialSlipIncrement(dof),
          _friction_reference_slip);
      std::array<ADReal, 2> lambda_t_plus_ctu;
      lambda_t_plus_ctu[0] = friction_lm_values[0] + c_t * *tangential_vel[0] * _dt;
      lambda_t_plus_ctu[1] = friction_lm_values[1] + c_t * *tangential_vel[1] * _dt;

      const ADReal friction_bound = mu_ad * max(0.0, lamdba_plus_cg);
      const ADRealVectorValue tangential_trial(lambda_t_plus_ctu[0], lambda_t_plus_ctu[1], 0.0);
      // The normalized projection keeps the tangential LM equation well-scaled when the
      // ARCTAN Coulomb bound vanishes. It may also benefit default Coulomb contact near
      // release, but is intentionally limited here to preserve legacy residual scaling
      // until that path has dedicated regression coverage.
      if (MetaPhysicL::raw_value(friction_bound) <= 0.0)
      {
        dof_residual = friction_lm_values[0];
        dof_residual_dir = friction_lm_values[1];
      }
      else
      {
        const ADReal projection_scale =
            max(friction_bound, Moose::Contact::tangentialSlipMagnitude(tangential_trial));
        dof_residual =
            friction_lm_values[0] - friction_bound / projection_scale * lambda_t_plus_ctu[0];
        dof_residual_dir =
            friction_lm_values[1] - friction_bound / projection_scale * lambda_t_plus_ctu[1];
      }
    }
    else
    {
      const ADReal mu_ad = base_mu;
      std::array<ADReal, 2> lambda_t_plus_ctu;
      lambda_t_plus_ctu[0] = friction_lm_values[0] + c_t * *tangential_vel[0] * _dt;
      lambda_t_plus_ctu[1] = friction_lm_values[1] + c_t * *tangential_vel[1] * _dt;

      const auto term_1_x = max(mu_ad * lamdba_plus_cg,
                                sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                                     lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt)) *
                            friction_lm_values[0];
      const auto term_1_y = max(mu_ad * lamdba_plus_cg,
                                sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                                     lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt)) *
                            friction_lm_values[1];
      const auto term_2_x = mu_ad * max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[0];
      const auto term_2_y = mu_ad * max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[1];

      dof_residual = term_1_x - term_2_x;
      dof_residual_dir = term_1_y - term_2_y;
    }
  }

  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{dof_residual}},
                          std::array<dof_id_type, 1>{{friction_dof_indices[0]}},
                          _friction_vars[0]->scalingFactor());
  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{dof_residual_dir}},
                          std::array<dof_id_type, 1>{{friction_dof_indices[1]}},
                          _friction_vars[1]->scalingFactor());
}

void
ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
  using std::abs, std::max;

  ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(dof);

  // Get friction LM
  const auto friction_dof_index = dof->dof_number(_sys.number(), _friction_vars[0]->number(), 0);
  const ADReal & tangential_vel = *_tangential_vel_ptr[0];
  ADReal friction_lm_value = (*_sys.currentSolution())(friction_dof_index);
  Moose::derivInsert(friction_lm_value.derivatives(), friction_dof_index, 1.);

  // Get normal LM
  const auto normal_dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  const ADReal & weighted_gap = *_weighted_gap_ptr;
  ADReal contact_pressure = (*_sys.currentSolution())(normal_dof_index);
  Moose::derivInsert(contact_pressure.derivatives(), normal_dof_index, 1.);
  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  const bool regularized =
      _friction_elastic_slip > 0.0 || _friction_coefficient_regularization !=
                                          Moose::Contact::FrictionCoefficientRegularization::NONE;
  ADReal base_mu;
  if (regularized && _has_friction_function)
  {
    const ADReal slip_rate =
        _dt > 0.0 ? _lm_weighted_velocities_uo->tangentialSlipIncrement(dof) / _dt : 0.0;
    base_mu = computeFrictionValue(contact_pressure, slip_rate, 0.0);
  }
  else
    base_mu = computeFrictionValue(contact_pressure, _dof_to_real_tangential_velocity[dof][0], 0.0);

  ADReal dof_residual;
  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon)
  {
    dof_residual = friction_lm_value;
  }
  else
  {
    const auto lambda_plus_cg = contact_pressure + c * weighted_gap;
    if (_friction_elastic_slip > 0.0)
    {
      const auto elastic_residual =
          computeElasticSlipResidual(dof, {{friction_lm_value, 0.0}}, base_mu, contact_pressure, 1);
      dof_residual = elastic_residual[0];
    }
    else if (_friction_coefficient_regularization ==
             Moose::Contact::FrictionCoefficientRegularization::ARCTAN_SLIP)
    {
      const ADReal mu_ad = Moose::Contact::arctanFrictionCoefficient(
          base_mu,
          _lm_weighted_velocities_uo->tangentialSlipIncrement(dof),
          _friction_reference_slip);
      const auto lambda_t_plus_ctu = friction_lm_value + c_t * tangential_vel * _dt;
      const ADReal friction_bound = mu_ad * max(0.0, lambda_plus_cg);
      if (MetaPhysicL::raw_value(friction_bound) <= 0.0)
        dof_residual = friction_lm_value;
      else
      {
        const ADRealVectorValue tangential_trial(lambda_t_plus_ctu, 0.0, 0.0);
        const ADReal projection_scale =
            max(friction_bound, Moose::Contact::tangentialSlipMagnitude(tangential_trial));
        dof_residual = friction_lm_value - friction_bound / projection_scale * lambda_t_plus_ctu;
      }
    }
    else
    {
      const ADReal mu_ad = base_mu;
      const auto lambda_t_plus_ctu = friction_lm_value + c_t * tangential_vel * _dt;
      const auto term_1 = max(mu_ad * lambda_plus_cg, abs(lambda_t_plus_ctu)) * friction_lm_value;
      const auto term_2 = mu_ad * max(0.0, lambda_plus_cg) * lambda_t_plus_ctu;

      dof_residual = term_1 - term_2;
    }
  }

  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{dof_residual}},
                          std::array<dof_id_type, 1>{{friction_dof_index}},
                          _friction_vars[0]->scalingFactor());
}

ADReal
ComputeFrictionalForceLMMechanicalContact::computeFrictionValue(
    const ADReal & contact_pressure,
    const ADReal & function_tangential_vel,
    const ADReal & function_tangential_vel_dir)
{
  using std::sqrt;

  // TODO: Introduce temperature dependence in the function. Do this when we have an example.
  ADReal mu_ad;

  if (!_has_friction_function)
    mu_ad = _mu;
  else
  {
    ADReal tangential_vel_magnitude =
        sqrt(function_tangential_vel * function_tangential_vel +
             function_tangential_vel_dir * function_tangential_vel_dir + 1.0e-24);
    mu_ad = _function_friction->value<ADReal>(0.0, contact_pressure, tangential_vel_magnitude, 0.0);
  }

  return mu_ad;
}

std::array<ADReal, 2>
ComputeFrictionalForceLMMechanicalContact::computeElasticSlipResidual(
    const DofObject * const dof,
    const std::array<ADReal, 2> & tangential_lm,
    const ADReal & friction_coefficient,
    const ADReal & contact_pressure,
    const unsigned int num_tangents)
{
  const auto state_it = _committed_elastic_slip_state.find(dof->id());
  const auto & frames = _lm_weighted_velocities_uo->contactFrames(dof);
  RealVectorValue committed_gap;
  if (state_it != _committed_elastic_slip_state.end())
  {
    committed_gap = state_it->second(0) * frames.material[0];
    if (num_tangents == 2)
      committed_gap += state_it->second(1) * frames.material[1];
  }

  const auto returned = Moose::Contact::elasticSlipReturnMap(
      committed_gap + _lm_weighted_velocities_uo->tangentialDisplacementIncrement(dof),
      friction_coefficient,
      contact_pressure,
      _friction_elastic_slip);

  std::array<ADReal, 2> residual = tangential_lm;
  if (MetaPhysicL::raw_value(returned.multiplier * returned.multiplier) == 0.0)
    return residual;

  // The tangential LM follows secondary-minus-primary slip; its corresponding physical traction
  // on the secondary side has the opposite sign.
  for (const auto direction : make_range(num_tangents))
    residual[direction] -= returned.multiplier * frames.constraint[direction];

  if (_assembly.computingResidual())
  {
    auto & state = _candidate_elastic_slip_state[dof->id()];
    state(0) = returned.elastic_gap * frames.material[0];
    state(1) = num_tangents == 2 ? returned.elastic_gap * frames.material[1] : 0.0;
  }
  return residual;
}
