//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFrictionalForceCartesianLMMechanicalContact.h"
#include "MortarContactUtils.h"
#include "DisplacedProblem.h"
#include "Assembly.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

registerMooseObject("ContactApp", ComputeFrictionalForceCartesianLMMechanicalContact);

namespace
{
const InputParameters &
assignVarsInParamsFriction(const InputParameters & params_in)
{
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  const auto & disp_x_name = ret.get<std::vector<VariableName>>("disp_x");
  if (disp_x_name.size() != 1)
    mooseError("We require that the disp_x parameter have exactly one coupled name");

  // We do this so we don't get any variable errors during MortarConstraint(Base) construction
  ret.set<VariableName>("secondary_variable") = disp_x_name[0];
  ret.set<VariableName>("primary_variable") = disp_x_name[0];

  return ret;
}
}

InputParameters
ComputeFrictionalForceCartesianLMMechanicalContact::validParams()
{
  InputParameters params = ComputeWeightedGapCartesianLMMechanicalContact::validParams();
  params.addClassDescription("Computes mortar frictional forces.");
  params.addParam<Real>("c_t", 1e0, "Numerical parameter for tangential constraints");
  params.addParam<Real>(
      "epsilon",
      1.0e-7,
      "Minimum value of contact pressure that will trigger frictional enforcement");
  params.addRangeCheckedParam<Real>(
      "mu", "mu > 0", "The friction coefficient for the Coulomb friction law");
  return params;
}

ComputeFrictionalForceCartesianLMMechanicalContact::
    ComputeFrictionalForceCartesianLMMechanicalContact(const InputParameters & parameters)
  : ComputeWeightedGapCartesianLMMechanicalContact(assignVarsInParamsFriction(parameters)),
    _c_t(getParam<Real>("c_t")),
    _secondary_x_dot(adCoupledDot("disp_x")),
    _primary_x_dot(adCoupledNeighborValueDot("disp_x")),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _mu(getParam<Real>("mu")),
    _epsilon(getParam<Real>("epsilon"))
{
}

void
ComputeFrictionalForceCartesianLMMechanicalContact::computeQpProperties()
{
  ComputeWeightedGapCartesianLMMechanicalContact::computeQpProperties();

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

  if (_has_disp_z)
    relative_velocity = {sec_x_dot - prim_x_dot, sec_y_dot - prim_y_dot, *sec_z_dot - *prim_z_dot};
  else
    relative_velocity = {sec_x_dot - prim_x_dot, sec_y_dot - prim_y_dot, 0.0};

  _qp_tangential_velocity_nodal = relative_velocity * (_JxW_msm[_qp] * _coord[_qp]);
}

void
ComputeFrictionalForceCartesianLMMechanicalContact::computeQpIProperties()
{
  ComputeWeightedGapCartesianLMMechanicalContact::computeQpIProperties();

  const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);
  // Get the _dof_to_weighted_tangential_velocity map
  const DofObject * const dof =
      _lm_vars[0]->isNodal() ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
                             : static_cast<const DofObject *>(_lower_secondary_elem);

  _dof_to_weighted_tangential_velocity[dof][0] +=
      _test[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[0][_i];

  // Get the _dof_to_weighted_tangential_velocity map for a second direction
  if (_has_disp_z)
    _dof_to_weighted_tangential_velocity[dof][1] +=
        _test[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[1][_i];
}

void
ComputeFrictionalForceCartesianLMMechanicalContact::residualSetup()
{
  ComputeWeightedGapCartesianLMMechanicalContact::residualSetup();
  _dof_to_weighted_tangential_velocity.clear();
}

void
ComputeFrictionalForceCartesianLMMechanicalContact::post()
{
  Moose::Mortar::Contact::communicateGaps(
      _dof_to_weighted_gap, _mesh, _nodal, _normalize_c, _communicator, false);
  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_weighted_tangential_velocity, _mesh, _nodal, _communicator, false);

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

    if (_has_disp_z)
      _tangential_vel_ptr[1] = &(pr.second[1]);

    enforceConstraintOnDof(dof);
  }
}

void
ComputeFrictionalForceCartesianLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  Moose::Mortar::Contact::communicateGaps(
      _dof_to_weighted_gap, _mesh, _nodal, _normalize_c, _communicator, false);
  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_weighted_tangential_velocity, _mesh, _nodal, _communicator, false);

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

    if (_has_disp_z)
      _tangential_vel_ptr[1] = &pr.second[1];

    enforceConstraintOnDof(dof);
  }
}

void
ComputeFrictionalForceCartesianLMMechanicalContact::enforceConstraintOnDof(
    const DofObject * const dof)
{
  const auto & weighted_gap = *_weighted_gap_ptr;
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  const auto dof_index_x = dof->dof_number(_sys.number(), _lm_vars[0]->number(), 0);
  const auto dof_index_y = dof->dof_number(_sys.number(), _lm_vars[1]->number(), 0);

  ADReal lm_x = (*_sys.currentSolution())(dof_index_x);
  ADReal lm_y = (*_sys.currentSolution())(dof_index_y);

  Moose::derivInsert(lm_x.derivatives(), dof_index_x, 1.);
  Moose::derivInsert(lm_y.derivatives(), dof_index_y, 1.);

  dof_id_type dof_index_z(-1);
  ADReal lm_z;
  if (_has_disp_z)
  {
    dof_index_z = dof->dof_number(_sys.number(), _lm_vars[2]->number(), 0);
    lm_z = (*_sys.currentSolution())(dof_index_z);
    Moose::derivInsert(lm_z.derivatives(), dof_index_z, 1.);
  }

  ADReal normal_pressure_value =
      lm_x * _dof_to_normal_vector[dof](0) + lm_y * _dof_to_normal_vector[dof](1);
  ADReal tangential_pressure_value =
      lm_x * _dof_to_tangent_vectors[dof][0](0) + lm_y * _dof_to_tangent_vectors[dof][0](1);

  ADReal tangential_pressure_value_dir;

  if (_has_disp_z)
  {
    normal_pressure_value += lm_z * _dof_to_normal_vector[dof](2);
    tangential_pressure_value += lm_z * _dof_to_tangent_vectors[dof][0](2);
    tangential_pressure_value_dir = lm_x * _dof_to_tangent_vectors[dof][1](0) +
                                    lm_y * _dof_to_tangent_vectors[dof][1](1) +
                                    lm_z * _dof_to_tangent_vectors[dof][1](2);
  }

  ADReal normal_dof_residual = std::min(normal_pressure_value, weighted_gap * c);
  ADReal tangential_dof_residual;
  ADReal tangential_dof_residual_dir;

  const ADReal & tangential_vel = *_tangential_vel_ptr[0];

  // Primal-dual active set strategy (PDASS)
  if (!_has_disp_z)
  {
    if (normal_pressure_value.value() < _epsilon)
      tangential_dof_residual = tangential_pressure_value;
    else
    {
      const auto term_1 =
          std::max(_mu * (normal_pressure_value + c * weighted_gap),
                   std::abs(tangential_pressure_value + c_t * tangential_vel * _dt)) *
          tangential_pressure_value;
      const auto term_2 = _mu * std::max(0.0, normal_pressure_value + c * weighted_gap) *
                          (tangential_pressure_value + c_t * tangential_vel * _dt);

      tangential_dof_residual = term_1 - term_2;
    }
  }
  else
  {
    if (normal_pressure_value.value() < _epsilon)
    {
      tangential_dof_residual = tangential_pressure_value;
      tangential_dof_residual_dir = tangential_pressure_value_dir;
    }
    else
    {
      const Real epsilon_sqrt = 1.0e-48;

      const auto lamdba_plus_cg = normal_pressure_value + c * weighted_gap;

      std::array<ADReal, 2> lambda_t_plus_ctu;
      lambda_t_plus_ctu[0] = tangential_pressure_value + c_t * (*_tangential_vel_ptr[0]) * _dt;
      lambda_t_plus_ctu[1] = tangential_pressure_value_dir + c_t * (*_tangential_vel_ptr[1]) * _dt;

      const auto term_1_x =
          std::max(_mu * lamdba_plus_cg,
                   std::sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                             lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt)) *
          tangential_pressure_value;

      const auto term_1_y =
          std::max(_mu * lamdba_plus_cg,
                   std::sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                             lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt)) *
          tangential_pressure_value_dir;

      const auto term_2_x = _mu * std::max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[0];

      const auto term_2_y = _mu * std::max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[1];

      tangential_dof_residual = term_1_x - term_2_x;
      tangential_dof_residual_dir = term_1_y - term_2_y;
    }
  }

  // Compute the friction coefficient (constant or function)

  // Get index for normal constraint.
  // We do this to get a decent Jacobian structure, which is key for the use of iterative solvers.
  // Using old normal vector to avoid changes in the Jacobian structure within one time step

  Real ny, nz;
  // Intially, use the current normal vector
  if (_dof_to_old_normal_vector[dof].norm() < TOLERANCE)
  {
    ny = _dof_to_normal_vector[dof](1);
    nz = _dof_to_normal_vector[dof](2);
  }
  else
  {
    ny = _dof_to_old_normal_vector[dof](1);
    nz = _dof_to_old_normal_vector[dof](2);
  }

  unsigned int component_normal = 0;

  // Consider constraint orientation to improve Jacobian structure
  const Real threshold_for_Jacobian = _has_disp_z ? 1.0 / std::sqrt(3.0) : 1.0 / std::sqrt(2.0);

  if (std::abs(ny) > threshold_for_Jacobian)
    component_normal = 1;
  else if (std::abs(nz) > threshold_for_Jacobian)
    component_normal = 2;

  libmesh_ignore(component_normal);

  _assembly.processResidualAndJacobian(
      normal_dof_residual,
      component_normal == 0 ? dof_index_x : (component_normal == 1 ? dof_index_y : dof_index_z),
      _vector_tags,
      _matrix_tags);

  _assembly.processResidualAndJacobian(
      tangential_dof_residual,
      (component_normal == 0 || component_normal == 2) ? dof_index_y : dof_index_x,
      _vector_tags,
      _matrix_tags);

  if (_has_disp_z)
    _assembly.processResidualAndJacobian(
        tangential_dof_residual_dir,
        (component_normal == 0 || component_normal == 1) ? dof_index_z : dof_index_x,
        _vector_tags,
        _matrix_tags);
}
