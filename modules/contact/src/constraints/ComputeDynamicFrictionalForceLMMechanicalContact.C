//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDynamicFrictionalForceLMMechanicalContact.h"
#include "DisplacedProblem.h"
#include "Assembly.h"
#include "Function.h"
#include "MortarContactUtils.h"
#include "ContactFrictionUtils.h"
#include "AutomaticMortarGeneration.h"

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

#include <limits>

registerMooseObject("ContactApp", ComputeDynamicFrictionalForceLMMechanicalContact);

InputParameters
ComputeDynamicFrictionalForceLMMechanicalContact::validParams()
{
  InputParameters params = ComputeDynamicWeightedGapLMMechanicalContact::validParams();
  params.addClassDescription("Computes the tangential frictional forces for dynamic simulations");
  params.addRequiredCoupledVar("friction_lm", "The frictional Lagrange's multiplier");
  params.addCoupledVar("friction_lm_dir",
                       "The frictional Lagrange's multiplier for an addtional direction.");
  params.addParam<FunctionName>(
      "function_friction",
      "Coupled function to evaluate friction with values from contact pressure and relative "
      "tangential velocities (from the previous step).");
  params.addParam<Real>("c_t", 1e0, "Numerical parameter for tangential constraints");
  params.addRangeCheckedParam<Real>(
      "epsilon",
      1.0e-7,
      "epsilon > 0",
      "Minimum value of contact pressure that will trigger frictional enforcement");
  params.addRangeCheckedParam<Real>(
      "mu", "mu >= 0", "The friction coefficient for the Coulomb friction law");
  params.addParam<MooseEnum>("friction_coefficient_regularization",
                             Moose::Contact::frictionCoefficientRegularizationOptions(),
                             "The regularization applied to the Coulomb friction coefficient.");
  params.addRangeCheckedParam<Real>(
      "friction_reference_slip",
      0.0,
      "friction_reference_slip >= 0",
      "Reference slip increment used by friction coefficient regularization.");
  params.addRangeCheckedParam<Real>(
      "friction_elastic_slip",
      0.0,
      "friction_elastic_slip >= 0",
      "Tangential elastic slip distance over which the Coulomb friction bound is reached.");
  return params;
}

ComputeDynamicFrictionalForceLMMechanicalContact::ComputeDynamicFrictionalForceLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeDynamicWeightedGapLMMechanicalContact(parameters),
    _c_t(getParam<Real>("c_t")),
    _secondary_x_dot(_secondary_var.adUDot()),
    _primary_x_dot(_primary_var.adUDotNeighbor()),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _epsilon(getParam<Real>("epsilon")),
    _mu(isParamValid("mu") ? getParam<Real>("mu") : std::numeric_limits<double>::quiet_NaN()),
    _friction_coefficient_regularization(
        getParam<MooseEnum>("friction_coefficient_regularization")
            .getEnum<Moose::Contact::FrictionCoefficientRegularization>()),
    _friction_reference_slip(getParam<Real>("friction_reference_slip")),
    _friction_elastic_slip(getParam<Real>("friction_elastic_slip")),
    _function_friction(isParamValid("function_friction") ? &getFunction("function_friction")
                                                         : nullptr),
    _has_friction_function(isParamValid("function_friction")),
    _3d(_has_disp_z)
{
  if (!_has_friction_function && !isParamValid("mu"))
    paramError("mu",
               "A coefficient of friction needs to be provided as a constant value or via a "
               "function.");

  if (_has_friction_function && isParamValid("mu"))
    paramError("mu",
               "Either provide a constant coefficient of friction or a function defining the "
               "coefficient of friction. Both inputs cannot be provided simultaneously.");

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

  _friction_vars.push_back(getVar("friction_lm", 0));

  if (_3d)
    _friction_vars.push_back(getVar("friction_lm_dir", 0));

  if (!_friction_vars[0]->isNodal())
    if (_friction_vars[0]->feType().order != static_cast<Order>(0))
      paramError(
          "friction_lm",
          "Frictional contact constraints only support elemental variables of CONSTANT order");

  // Request the old solution state in unison
  _sys.solutionOld();
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::computeQpProperties()
{
  // Compute the value of _qp_gap
  ComputeDynamicWeightedGapLMMechanicalContact::computeQpProperties();

  // It appears that the relative velocity between weighted gap and this class have a sign
  // difference
  _qp_tangential_velocity_nodal = -_relative_velocity * (_JxW_msm[_qp] * _coord[_qp]);
  _qp_real_tangential_velocity_nodal = -_relative_velocity;
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::computeQpIProperties()
{
  // Get the _dof_to_weighted_gap map
  ComputeDynamicWeightedGapLMMechanicalContact::computeQpIProperties();

  const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);

  // Get the _dof_to_weighted_tangential_velocity map
  const DofObject * const dof =
      _friction_vars[0]->isNodal()
          ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
          : static_cast<const DofObject *>(_lower_secondary_elem);

  _dof_to_weighted_tangential_velocity[dof][0] +=
      _test[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[0][_i];

  _dof_to_real_tangential_velocity[dof][0] +=
      _test[_i][_qp] * _qp_real_tangential_velocity_nodal * nodal_tangents[0][_i];

  // Get the _dof_to_weighted_tangential_velocity map for a second direction
  if (_3d)
  {
    _dof_to_weighted_tangential_velocity[dof][1] +=
        _test[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[1][_i];

    _dof_to_real_tangential_velocity[dof][1] +=
        _test[_i][_qp] * _qp_real_tangential_velocity_nodal * nodal_tangents[1][_i];
  }
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::residualSetup()
{
  // Clear both maps
  ComputeDynamicWeightedGapLMMechanicalContact::residualSetup();
  _dof_to_weighted_tangential_velocity.clear();
  _dof_to_real_tangential_velocity.clear();
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::timestepSetup()
{

  ComputeDynamicWeightedGapLMMechanicalContact::timestepSetup();

  _dof_to_old_real_tangential_velocity.clear();

  for (const auto & [dof, real_tangential_velocity] : _dof_to_real_tangential_velocity)
    _dof_to_old_real_tangential_velocity.emplace(
        dof,
        std::array<Real, 2>{{MetaPhysicL::raw_value(real_tangential_velocity[0]),
                             MetaPhysicL::raw_value(real_tangential_velocity[1])}});
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::post()
{
  ComputeDynamicWeightedGapLMMechanicalContact::post();

  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_weighted_tangential_velocity, _mesh, _nodal, _communicator, false);

  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_real_tangential_velocity, _mesh, _nodal, _communicator, false);

  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    if (dof->processor_id() != this->processor_id())
      continue;

    // Use always weighted gap for dynamic PDASS. Omit the dynamic weighted gap approach that is
    // used in normal contact where the discretized gap velocity is enforced if a node has
    // identified to be into contact.

    _weighted_gap_ptr = &_dof_to_weighted_gap[dof].first;
    _normalization_ptr = &_dof_to_weighted_gap[dof].second;
    _tangential_vel_ptr[0] = &(pr.second[0]);

    if (_3d)
    {
      _tangential_vel_ptr[1] = &(pr.second[1]);
      enforceConstraintOnDof3d(dof);
    }
    else
      enforceConstraintOnDof(dof);
  }
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  ComputeDynamicWeightedGapLMMechanicalContact::incorrectEdgeDroppingPost(inactive_lm_nodes);

  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_weighted_tangential_velocity, _mesh, _nodal, _communicator, false);

  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_real_tangential_velocity, _mesh, _nodal, _communicator, false);

  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    // If node inactive, skip
    if ((inactive_lm_nodes.find(static_cast<const Node *>(dof)) != inactive_lm_nodes.end()) ||
        (dof->processor_id() != this->processor_id()))
      continue;

    // Use always weighted gap for dynamic PDASS
    _weighted_gap_ptr = &_dof_to_weighted_gap[dof].first;
    _normalization_ptr = &_dof_to_weighted_gap[dof].second;
    _tangential_vel_ptr[0] = &pr.second[0];

    if (_3d)
    {
      _tangential_vel_ptr[1] = &pr.second[1];
      enforceConstraintOnDof3d(dof);
    }
    else
      enforceConstraintOnDof(dof);
  }
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::enforceConstraintOnDof3d(
    const DofObject * const dof)
{
  using std::max, std::sqrt;

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

  const Real contact_pressure_old = _sys.solutionOld()(normal_dof_index);

  // Compute the friction coefficient (constant or function)
  const auto & current_real_tangential_velocity =
      libmesh_map_find(_dof_to_real_tangential_velocity, dof);
  const auto old_real_tangential_velocity_it = _dof_to_old_real_tangential_velocity.find(dof);
  const std::array<Real, 2> current_function_real_tangential_velocity{
      {MetaPhysicL::raw_value(current_real_tangential_velocity[0]),
       MetaPhysicL::raw_value(current_real_tangential_velocity[1])}};
  const auto & function_real_tangential_velocity =
      old_real_tangential_velocity_it != _dof_to_old_real_tangential_velocity.end()
          ? old_real_tangential_velocity_it->second
          : current_function_real_tangential_velocity;
  const ADReal slip_increment =
      sqrt(current_real_tangential_velocity[0] * current_real_tangential_velocity[0] +
           current_real_tangential_velocity[1] * current_real_tangential_velocity[1] + 1.0e-24) *
      _dt;
  ADReal mu_ad = computeFrictionValue(contact_pressure_old,
                                      function_real_tangential_velocity[0],
                                      function_real_tangential_velocity[1],
                                      slip_increment);

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
    const Real epsilon_sqrt = 1.0e-48;

    const auto lamdba_plus_cg = contact_pressure + c * weighted_gap;
    const auto normal_bound = max(0.0, lamdba_plus_cg);
    const auto friction_bound = mu_ad * normal_bound;
    const auto tangential_compliance =
        _friction_elastic_slip > 0.0 ? _friction_elastic_slip / (friction_bound + _epsilon) : 0.0;

    std::array<ADReal, 2> lambda_t_plus_ctu;
    lambda_t_plus_ctu[0] =
        friction_lm_values[0] +
        c_t * (*tangential_vel[0] * _dt - tangential_compliance * friction_lm_values[0]);
    lambda_t_plus_ctu[1] =
        friction_lm_values[1] +
        c_t * (*tangential_vel[1] * _dt - tangential_compliance * friction_lm_values[1]);

    const auto tangential_trial_norm =
        sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
             lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt);

    const auto term_1_x = max(friction_bound, tangential_trial_norm) * friction_lm_values[0];

    const auto term_1_y = max(friction_bound, tangential_trial_norm) * friction_lm_values[1];

    const auto term_2_x = friction_bound * lambda_t_plus_ctu[0];

    const auto term_2_y = friction_bound * lambda_t_plus_ctu[1];

    dof_residual = term_1_x - term_2_x;
    dof_residual_dir = term_1_y - term_2_y;
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
ComputeDynamicFrictionalForceLMMechanicalContact::enforceConstraintOnDof(
    const DofObject * const dof)
{
  using std::max, std::abs, std::sqrt;

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

  const Real contact_pressure_old = _sys.solutionOld()(normal_dof_index);

  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  // Compute the friction coefficient (constant or function)
  const auto & current_real_tangential_velocity =
      libmesh_map_find(_dof_to_real_tangential_velocity, dof);
  const auto old_real_tangential_velocity_it = _dof_to_old_real_tangential_velocity.find(dof);
  const std::array<Real, 2> current_function_real_tangential_velocity{
      {MetaPhysicL::raw_value(current_real_tangential_velocity[0]),
       MetaPhysicL::raw_value(current_real_tangential_velocity[1])}};
  const auto & function_real_tangential_velocity =
      old_real_tangential_velocity_it != _dof_to_old_real_tangential_velocity.end()
          ? old_real_tangential_velocity_it->second
          : current_function_real_tangential_velocity;
  const ADReal slip_increment =
      sqrt(current_real_tangential_velocity[0] * current_real_tangential_velocity[0] + 1.0e-24) *
      _dt;
  ADReal mu_ad = computeFrictionValue(
      contact_pressure_old, function_real_tangential_velocity[0], 0.0, slip_increment);

  ADReal dof_residual;
  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon)
    dof_residual = friction_lm_value;
  else
  {
    const auto lambda_plus_cg = contact_pressure + c * weighted_gap;
    const auto normal_bound = max(0.0, lambda_plus_cg);
    const auto friction_bound = mu_ad * normal_bound;
    const auto tangential_compliance =
        _friction_elastic_slip > 0.0 ? _friction_elastic_slip / (friction_bound + _epsilon) : 0.0;
    const auto lambda_t_plus_ctu =
        friction_lm_value +
        c_t * (tangential_vel * _dt - tangential_compliance * friction_lm_value);

    const auto term_1 = max(friction_bound, abs(lambda_t_plus_ctu)) * friction_lm_value;
    const auto term_2 = friction_bound * lambda_t_plus_ctu;

    dof_residual = term_1 - term_2;
  }

  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{dof_residual}},
                          std::array<dof_id_type, 1>{{friction_dof_index}},
                          _friction_vars[0]->scalingFactor());
}

ADReal
ComputeDynamicFrictionalForceLMMechanicalContact::computeFrictionValue(
    const ADReal & contact_pressure,
    const Real & function_tangential_vel,
    const Real & function_tangential_vel_dir,
    const ADReal & slip_increment)
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

  return Moose::Contact::regularizedFrictionCoefficient(
      mu_ad, slip_increment, _friction_coefficient_regularization, _friction_reference_slip);
}
