//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "DualRealOps.h"

#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
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
  return params;
}

ComputeFrictionalForceLMMechanicalContact::ComputeFrictionalForceLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeWeightedGapLMMechanicalContact(parameters),
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
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpProperties()
{
  // Compute the value of _qp_gap
  ComputeWeightedGapLMMechanicalContact::computeQpProperties();

  // Trim derivatives
  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

  std::array<const MooseVariable *, 3> var_array{{_disp_x_var, _disp_y_var, _disp_z_var}};
  std::array<ADReal, 3> primary_disp_dot{
      {_primary_x_dot[_qp], _primary_y_dot[_qp], _has_disp_z ? (*_primary_z_dot)[_qp] : 0}};
  std::array<ADReal, 3> secondary_disp_dot{
      {_secondary_x_dot[_qp], _secondary_y_dot[_qp], _has_disp_z ? (*_secondary_z_dot)[_qp] : 0}};

  trimInteriorNodeDerivatives(primary_ip_lowerd_map, var_array, primary_disp_dot, false);
  trimInteriorNodeDerivatives(secondary_ip_lowerd_map, var_array, secondary_disp_dot, true);

  const ADReal & prim_x_dot = primary_disp_dot[0];
  const ADReal & prim_y_dot = primary_disp_dot[1];
  const ADReal * prim_z_dot = nullptr;
  if (_has_disp_z)
    prim_z_dot = &primary_disp_dot[2];

  const ADReal & sec_x_dot = secondary_disp_dot[0];
  const ADReal & sec_y_dot = secondary_disp_dot[1];
  const ADReal * sec_z_dot = nullptr;
  if (_has_disp_z)
    sec_z_dot = &secondary_disp_dot[2];

  // Build relative velocity vector
  ADRealVectorValue relative_velocity;

  if (_3d)
    relative_velocity = {sec_x_dot - prim_x_dot, sec_y_dot - prim_y_dot, *sec_z_dot - *prim_z_dot};
  else
    relative_velocity = {sec_x_dot - prim_x_dot, sec_y_dot - prim_y_dot, 0.0};

  // Compute integration point quantity for constraint enforcement
  if (_interpolate_normals)
  {
    _qp_tangential_velocity[0] =
        relative_velocity * (_tangents[_qp][0] * _JxW_msm[_qp] * _coord[_qp]);
    _qp_real_tangential_velocity[0] = relative_velocity * (_tangents[_qp][0]);

    if (_3d)
    {
      _qp_tangential_velocity[1] =
          relative_velocity * (_tangents[_qp][1] * _JxW_msm[_qp] * _coord[_qp]);
      _qp_real_tangential_velocity[1] = relative_velocity * (_tangents[_qp][1]);
    }
  }
  else
  {
    _qp_real_tangential_velocity_nodal = relative_velocity;
    _qp_tangential_velocity_nodal = relative_velocity * (_JxW_msm[_qp] * _coord[_qp]);
  }
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpIProperties()
{
  // Get the _dof_to_weighted_gap map
  ComputeWeightedGapLMMechanicalContact::computeQpIProperties();

  const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);
  // Get the _dof_to_weighted_tangential_velocity map
  const DofObject * const dof =
      _friction_vars[0]->isNodal()
          ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
          : static_cast<const DofObject *>(_lower_secondary_elem);

  if (_interpolate_normals)
  {
    _dof_to_weighted_tangential_velocity[dof][0] += _test[_i][_qp] * _qp_tangential_velocity[0];
    _dof_to_real_tangential_velocity[dof][0] += _test[_i][_qp] * _qp_real_tangential_velocity[0];
  }
  else
  {
    _dof_to_weighted_tangential_velocity[dof][0] +=
        _test[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[0][_i];
    _dof_to_real_tangential_velocity[dof][0] +=
        _test[_i][_qp] * _qp_real_tangential_velocity_nodal * nodal_tangents[0][_i];
  }

  // Get the _dof_to_weighted_tangential_velocity map for a second direction
  if (_3d)
  {
    if (_interpolate_normals)
    {
      _dof_to_weighted_tangential_velocity[dof][1] += _test[_i][_qp] * _qp_tangential_velocity[1];
      _dof_to_real_tangential_velocity[dof][1] += _test[_i][_qp] * _qp_real_tangential_velocity[1];
    }
    else
    {
      _dof_to_weighted_tangential_velocity[dof][1] +=
          _test[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[1][_i];

      _dof_to_real_tangential_velocity[dof][1] +=
          _test[_i][_qp] * _qp_real_tangential_velocity_nodal * nodal_tangents[1][_i];
    }
  }
}

void
ComputeFrictionalForceLMMechanicalContact::residualSetup()
{
  // Clear both maps
  ComputeWeightedGapLMMechanicalContact::residualSetup();
  _dof_to_weighted_tangential_velocity.clear();
  _dof_to_real_tangential_velocity.clear();
}

void
ComputeFrictionalForceLMMechanicalContact::post()
{
  Moose::Mortar::Contact::communicateGaps(_dof_to_weighted_gap,
                                          this->processor_id(),
                                          _mesh,
                                          _nodal,
                                          _normalize_c,
                                          _communicator,
                                          false);
  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_weighted_tangential_velocity, this->processor_id(), _mesh, _nodal, _communicator);
  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_real_tangential_velocity, this->processor_id(), _mesh, _nodal, _communicator);

  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    if (dof->processor_id() != this->processor_id())
      continue;

    auto & weighted_gap_pr = _dof_to_weighted_gap[dof];
    _weighted_gap_ptr = &weighted_gap_pr.first;
    _normalization_ptr = &weighted_gap_pr.second;
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
ComputeFrictionalForceLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  Moose::Mortar::Contact::communicateGaps(_dof_to_weighted_gap,
                                          this->processor_id(),
                                          _mesh,
                                          _nodal,
                                          _normalize_c,
                                          _communicator,
                                          false);
  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_weighted_tangential_velocity, this->processor_id(), _mesh, _nodal, _communicator);
  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_real_tangential_velocity, this->processor_id(), _mesh, _nodal, _communicator);

  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    // If node inactive, skip
    if ((inactive_lm_nodes.find(static_cast<const Node *>(dof)) != inactive_lm_nodes.end()) ||
        (dof->processor_id() != this->processor_id()))
      continue;

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

  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  // Compute the friction coefficient (constant or function)
  ADReal mu_ad = computeFrictionValue(contact_pressure,
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
    // Espilon to avoid automatic differentiation singularity
    const Real epsilon_sqrt = 1.0e-48;

    const auto lamdba_plus_cg = contact_pressure + c * weighted_gap;
    std::array<ADReal, 2> lambda_t_plus_ctu;
    lambda_t_plus_ctu[0] = friction_lm_values[0] + c_t * *tangential_vel[0] * _dt;
    lambda_t_plus_ctu[1] = friction_lm_values[1] + c_t * *tangential_vel[1] * _dt;

    const auto term_1_x =
        std::max(mu_ad * lamdba_plus_cg,
                 std::sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                           lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt)) *
        friction_lm_values[0];

    const auto term_1_y =
        std::max(mu_ad * lamdba_plus_cg,
                 std::sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                           lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt)) *
        friction_lm_values[1];

    const auto term_2_x = mu_ad * std::max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[0];

    const auto term_2_y = mu_ad * std::max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[1];

    dof_residual = term_1_x - term_2_x;
    dof_residual_dir = term_1_y - term_2_y;
  }

  _assembly.processResidualAndJacobian(
      dof_residual, friction_dof_indices[0], _vector_tags, _matrix_tags);
  _assembly.processResidualAndJacobian(
      dof_residual_dir, friction_dof_indices[1], _vector_tags, _matrix_tags);
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

  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  // Compute the friction coefficient (constant or function)
  ADReal mu_ad =
      computeFrictionValue(contact_pressure, _dof_to_real_tangential_velocity[dof][0], 0.0);

  ADReal dof_residual;
  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon)
    dof_residual = friction_lm_value;
  else
  {
    const auto term_1 = std::max(mu_ad * (contact_pressure + c * weighted_gap),
                                 std::abs(friction_lm_value + c_t * tangential_vel * _dt)) *
                        friction_lm_value;
    const auto term_2 = mu_ad * std::max(0.0, contact_pressure + c * weighted_gap) *
                        (friction_lm_value + c_t * tangential_vel * _dt);

    dof_residual = term_1 - term_2;
  }

  _assembly.processResidualAndJacobian(
      dof_residual, friction_dof_index, _vector_tags, _matrix_tags);
}

ADReal
ComputeFrictionalForceLMMechanicalContact::computeFrictionValue(const ADReal & contact_pressure,
                                                                const ADReal & tangential_vel,
                                                                const ADReal & tangential_vel_dir)
{
  // TODO: Introduce temperature dependence in the function. Do this when we have an example.
  ADReal mu_ad;

  if (!_has_friction_function)
    mu_ad = _mu;
  else
  {
    ADReal tangential_vel_magnitude = std::sqrt(tangential_vel * tangential_vel +
                                                tangential_vel_dir * tangential_vel_dir + 1.0e-24);
    mu_ad = _function_friction->value<ADReal>(0.0, contact_pressure, tangential_vel_magnitude, 0.0);
  }

  return mu_ad;
}
