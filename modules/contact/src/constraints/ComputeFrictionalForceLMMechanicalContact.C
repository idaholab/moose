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
#include "NonlinearSystemBase.h"

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
      "Legacy separation threshold retained for input compatibility; quasistatic friction uses "
      "the exact projection degeneracy guard.");
  params.addRangeCheckedParam<Real>(
      "mu", "mu > 0", "The friction coefficient for the Coulomb friction law");
  params.addRequiredParam<UserObjectName>("weighted_velocities_uo",
                                          "The weighted tangential velocities user object.");
  params.addParam<bool>(
      "dynamic_c_t", false, "Use the physical normal stiffness as the per-node tangential scale.");
  MooseEnum friction_ncp_formulation("hueber_stadler_wohlmuth alart_curnier",
                                     "hueber_stadler_wohlmuth");
  friction_ncp_formulation.addDocumentation(
      "hueber_stadler_wohlmuth", "Use the degree-two scaled projection residual (default).");
  friction_ncp_formulation.addDocumentation("alart_curnier",
                                            "Use the degree-one projection residual.");
  params.addParam<MooseEnum>("friction_ncp_formulation",
                             friction_ncp_formulation,
                             "Nonlinear complementarity formulation for Coulomb friction.");
  return params;
}

ComputeFrictionalForceLMMechanicalContact::ComputeFrictionalForceLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeWeightedGapLMMechanicalContact(parameters),
    _weighted_velocities_uo(
        getUserObject<LMWeightedVelocitiesUserObject>("weighted_velocities_uo")),
    _c_t(getParam<Real>("c_t")),
    _dynamic_c_t(getParam<bool>("dynamic_c_t")),
    _friction_ncp_formulation(getParam<MooseEnum>("friction_ncp_formulation")),
    _secondary_x_dot(_secondary_var.adUDot()),
    _primary_x_dot(_primary_var.adUDotNeighbor()),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _mu(isParamValid("function_friction") ? std::numeric_limits<double>::quiet_NaN()
                                          : getParam<Real>("mu")),
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

  _friction_vars.push_back(getVar("friction_lm", 0));

  if (_3d)
    _friction_vars.push_back(getVar("friction_lm_dir", 0));

  if (!_friction_vars[0]->isNodal())
    if (_friction_vars[0]->feType().order != static_cast<Order>(0))
      paramError(
          "friction_lm",
          "Frictional contact constraints only support elemental variables of CONSTANT order");

  if (_dynamic_c_t && !_use_derived_c_normal)
    paramError("dynamic_c_t",
               "Physical tangential scaling requires physical normal contact scaling.");

  if (!_dynamic_c_t && (!std::isfinite(_c_t) || _c_t <= 0.0))
    paramError("c_t",
               "The user-supplied tangential contact pressure scale must be positive and finite.");
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

  _fe_problem.getNonlinearSystemBase(_sys.number()).closeKSPRightDiagonalScale();
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

  _fe_problem.getNonlinearSystemBase(_sys.number()).closeKSPRightDiagonalScale();
}

void
ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnDof3d(const DofObject * const dof)
{
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

  const Real normalization = contactNormalization();
  const Real normal_scale = normalContactScale(dof);
  const Real tangential_scale = tangentialContactScale(dof);
  const Real c = normal_scale / normalization;
  const Real c_t = tangential_scale / normalization;

  // Compute the friction coefficient (constant or function)
  ADReal mu_ad = computeFrictionValue(contact_pressure,
                                      _dof_to_real_tangential_velocity[dof][0],
                                      _dof_to_real_tangential_velocity[dof][1]);

  const Real equation_scaling = _friction_vars[0]->scalingFactor();
  const Real equation_scaling_dir = _friction_vars[1]->scalingFactor();

  auto & nonlinear_system = _fe_problem.getNonlinearSystemBase(_sys.number());
  for (const auto i : make_range(num_tangents))
    nonlinear_system.setKSPRightDiagonalScale(friction_dof_indices[i], tangential_scale);

  const ADReal augmented_normal_pressure =
      Moose::Mortar::Contact::augmentedNormalPressure(contact_pressure, c * weighted_gap);
  const ADReal friction_limit =
      Moose::Mortar::Contact::coulombFrictionRadius(mu_ad, augmented_normal_pressure);
  const std::array<ADReal, 2> augmented_tangential_pressure = {
      friction_lm_values[0] + c_t * *tangential_vel[0] * _dt,
      friction_lm_values[1] + c_t * *tangential_vel[1] * _dt};
  const auto friction_residual =
      _friction_ncp_formulation == "alart_curnier"
          ? Moose::Mortar::Contact::alartCurnierFrictionResidual(
                friction_lm_values, augmented_tangential_pressure, friction_limit)
          : Moose::Mortar::Contact::hueberStadlerWohlmuthFrictionResidual(
                friction_lm_values, augmented_tangential_pressure, friction_limit);
  const Real formulation_row_scale =
      _friction_ncp_formulation == "alart_curnier" ? 1.0 : 1.0 / tangential_scale;

  const ADReal dof_residual =
      equationCompensation(*_friction_vars[0]) * formulation_row_scale * friction_residual[0];
  const ADReal dof_residual_dir =
      equationCompensation(*_friction_vars[1]) * formulation_row_scale * friction_residual[1];

  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{dof_residual}},
                          std::array<dof_id_type, 1>{{friction_dof_indices[0]}},
                          equation_scaling);
  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{dof_residual_dir}},
                          std::array<dof_id_type, 1>{{friction_dof_indices[1]}},
                          equation_scaling_dir);
}

void
ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
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

  const Real normalization = contactNormalization();
  const Real normal_scale = normalContactScale(dof);
  const Real tangential_scale = tangentialContactScale(dof);
  const Real c = normal_scale / normalization;
  const Real c_t = tangential_scale / normalization;

  // Compute the friction coefficient (constant or function)
  ADReal mu_ad =
      computeFrictionValue(contact_pressure, _dof_to_real_tangential_velocity[dof][0], 0.0);

  const Real equation_scaling = _friction_vars[0]->scalingFactor();
  _fe_problem.getNonlinearSystemBase(_sys.number())
      .setKSPRightDiagonalScale(friction_dof_index, tangential_scale);

  const ADReal augmented_normal_pressure =
      Moose::Mortar::Contact::augmentedNormalPressure(contact_pressure, c * weighted_gap);
  const ADReal friction_limit =
      Moose::Mortar::Contact::coulombFrictionRadius(mu_ad, augmented_normal_pressure);
  const std::array<ADReal, 1> tangential_pressure = {friction_lm_value};
  const std::array<ADReal, 1> augmented_tangential_pressure = {friction_lm_value +
                                                               c_t * tangential_vel * _dt};
  const auto friction_residual =
      _friction_ncp_formulation == "alart_curnier"
          ? Moose::Mortar::Contact::alartCurnierFrictionResidual(
                tangential_pressure, augmented_tangential_pressure, friction_limit)
          : Moose::Mortar::Contact::hueberStadlerWohlmuthFrictionResidual(
                tangential_pressure, augmented_tangential_pressure, friction_limit);
  const Real formulation_row_scale =
      _friction_ncp_formulation == "alart_curnier" ? 1.0 : 1.0 / tangential_scale;
  const ADReal dof_residual =
      equationCompensation(*_friction_vars[0]) * formulation_row_scale * friction_residual[0];

  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{dof_residual}},
                          std::array<dof_id_type, 1>{{friction_dof_index}},
                          equation_scaling);
}

Real
ComputeFrictionalForceLMMechanicalContact::tangentialContactScale(const DofObject * const dof) const
{
  const Real scale =
      _dynamic_c_t ? normalContactScale(dof) : _c_t * (_normalize_c ? 1.0 : contactNormalization());
  if (!std::isfinite(scale) || scale <= 0.0)
    mooseError("Mortar contact requires positive, finite nodal tangential pressure scales.");
  return scale;
}

ADReal
ComputeFrictionalForceLMMechanicalContact::computeFrictionValue(const ADReal & contact_pressure,
                                                                const ADReal & tangential_vel,
                                                                const ADReal & tangential_vel_dir)
{
  using std::sqrt;

  // TODO: Introduce temperature dependence in the function. Do this when we have an example.
  ADReal mu_ad;

  if (!_has_friction_function)
    mu_ad = _mu;
  else
  {
    ADReal tangential_vel_magnitude =
        sqrt(tangential_vel * tangential_vel + tangential_vel_dir * tangential_vel_dir + 1.0e-24);
    mu_ad = _function_friction->value<ADReal>(0.0, contact_pressure, tangential_vel_magnitude, 0.0);
  }

  return mu_ad;
}
