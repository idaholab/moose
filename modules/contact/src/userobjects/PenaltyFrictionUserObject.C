//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyFrictionUserObject.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "MathUtils.h"
#include "MortarContactUtils.h"

registerMooseObject("ContactApp", PenaltyFrictionUserObject);

InputParameters
PenaltyFrictionUserObject::validParams()
{
  InputParameters params = WeightedVelocitiesUserObject::validParams();
  params.addClassDescription(
      "Computes the mortar frictional contact force via a penalty approach.");
  params.addRequiredParam<Real>("penalty", "The penalty factor for normal interaction.");
  params.addParam<Real>("penalty_friction",
                        "The penalty factor for frictional interaction. If not provide, the normal "
                        "penalty factor is also used for the frictional problem.");
  params.addRequiredParam<Real>("friction_coefficient",
                                "The friction coefficient ruling Coulomb friction equations.");
  return params;
}

PenaltyFrictionUserObject::PenaltyFrictionUserObject(const InputParameters & parameters)
  : WeightedVelocitiesUserObject(parameters),
    _penalty(getParam<Real>("penalty")),
    _penalty_friction(isParamValid("penalty_friction") ? getParam<Real>("penalty_friction")
                                                       : getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient"))
{
  auto check_type = [this](const auto & var, const auto & var_name)
  {
    if (!var.isNodal())
      paramError(var_name,
                 "The displacement variables must have degrees of freedom exclusively on "
                 "nodes, e.g. they should probably be of finite element type 'Lagrange'.");
  };
  check_type(*_disp_x_var, "disp_x");
  check_type(*_disp_y_var, "disp_y");
  if (_has_disp_z)
    check_type(*_disp_z_var, "disp_z");
}

const VariableTestValue &
PenaltyFrictionUserObject::test() const
{
  return _disp_x_var->phiLower();
}

const ADVariableValue &
PenaltyFrictionUserObject::contactPressure() const
{
  return _contact_force;
}

const ADVariableValue &
PenaltyFrictionUserObject::contactTangentialPressureDirOne() const
{
  return _frictional_contact_force_one;
}

const ADVariableValue &
PenaltyFrictionUserObject::contactTangentialPressureDirTwo() const
{
  return _frictional_contact_force_two;
}

void
PenaltyFrictionUserObject::timestepSetup()
{
  WeightedVelocitiesUserObject::timestepSetup();

  _dof_to_old_weighted_gap.clear();
  _dof_to_old_real_tangential_velocity.clear();
  _dof_to_old_accumulated_slip.clear();
  _dof_to_old_normal_pressure.clear();

  for (auto & map_pr : _dof_to_weighted_gap)
    _dof_to_old_weighted_gap.emplace(map_pr.first, std::move(map_pr.second.first));

  for (auto & map_pr : _dof_to_real_tangential_velocity)
    _dof_to_old_real_tangential_velocity.emplace(map_pr);

  for (auto & map_pr : _dof_to_normal_pressure)
    _dof_to_old_normal_pressure.emplace(map_pr.first, MetaPhysicL::raw_value(map_pr.second));

  for (auto & map_pr : _dof_to_accumulated_slip)
  {
    if (libmesh_map_find(_dof_to_old_normal_pressure, map_pr.first) >
        TOLERANCE * TOLERANCE) // Only if it had normal pressure. Otherwise, restart count
      _dof_to_old_accumulated_slip.emplace(map_pr);
    else
      _dof_to_old_accumulated_slip[map_pr.first] = {0.0, 0.0};
  }
}

void
PenaltyFrictionUserObject::initialize()
{
  WeightedVelocitiesUserObject::initialize();

  _dof_to_normal_pressure.clear();
  _dof_to_accumulated_slip.clear();
  _dof_to_frictional_pressure.clear();
}

Real
PenaltyFrictionUserObject::getNormalContactPressure(const Node * const node) const
{
  const auto it = _dof_to_normal_pressure.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_normal_pressure.end())
    return MetaPhysicL::raw_value(it->second);
  else
    return 0.0;
}

Real
PenaltyFrictionUserObject::getFrictionalContactPressure(const Node * const node,
                                                        const unsigned int component) const
{
  const auto it = _dof_to_frictional_pressure.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_frictional_pressure.end())
    return MetaPhysicL::raw_value(it->second[component]);
  else
    return 0.0;
}

Real
PenaltyFrictionUserObject::getAccumulatedSlip(const Node * const node,
                                              const unsigned int component) const
{
  const auto it = _dof_to_accumulated_slip.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_accumulated_slip.end())
    return MetaPhysicL::raw_value(it->second[component]);
  else
    return 0.0;
}

Real
PenaltyFrictionUserObject::getTangentialVelocity(const Node * const node,
                                                 const unsigned int component) const
{

  const auto it = _dof_to_real_tangential_velocity.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_real_tangential_velocity.end())
    return MetaPhysicL::raw_value(it->second[component]);
  else
    return 0.0;
}

void
PenaltyFrictionUserObject::reinit()
{
  // Normal contact pressure with penalty
  _contact_force.resize(_qrule_msm->n_points());
  for (const auto qp : make_range(_qrule_msm->n_points()))
    _contact_force[qp] = 0;

  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);
    const auto & weighted_gap =
        libmesh_map_find(_dof_to_weighted_gap, static_cast<const DofObject *>(node)).first;
    const auto weighted_gap_for_calc = weighted_gap < 0 ? -weighted_gap : ADReal(0);
    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(_qrule_msm->n_points()))
    {
      _contact_force[qp] += (test_i[qp] * _penalty) * weighted_gap_for_calc;
      _dof_to_normal_pressure[static_cast<const DofObject *>(node)] =
          _penalty * weighted_gap_for_calc;
    }
  }

  // Frictional pressure (2D)
  _frictional_contact_force_one.resize(_qrule_msm->n_points());
  for (const auto qp : make_range(_qrule_msm->n_points()))
    _frictional_contact_force_one[qp] = 0;

  // Frictional pressure (3D only)
  _frictional_contact_force_two.resize(_qrule_msm->n_points());
  for (const auto qp : make_range(_qrule_msm->n_points()))
    _frictional_contact_force_two[qp] = 0;

  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    const auto & slip_vel_one =
        libmesh_map_find(_dof_to_real_tangential_velocity, static_cast<const DofObject *>(node))[0];
    const auto & slip_vel_two =
        libmesh_map_find(_dof_to_real_tangential_velocity, static_cast<const DofObject *>(node))[1];

    // Get accumulated slip in both directions
    Real slip_direction_one(0.0);
    Real slip_direction_two(0.0);

    if (_dof_to_old_accumulated_slip.size() > 0)
    {
      slip_direction_one = MetaPhysicL::raw_value(
          _dof_to_old_accumulated_slip[static_cast<const DofObject *>(node)][0]);

      if (_has_disp_z)
        slip_direction_two = MetaPhysicL::raw_value(
            _dof_to_old_accumulated_slip[static_cast<const DofObject *>(node)][1]);
    }
    // Get current accumulated slip in both directions
    if (_dof_to_normal_pressure[static_cast<const DofObject *>(node)] > TOLERANCE * TOLERANCE)
    {
      _dof_to_accumulated_slip[static_cast<const DofObject *>(node)][0] =
          slip_direction_one + std::abs(slip_vel_one) * _fe_problem.dt();
      if (_has_disp_z)
        _dof_to_accumulated_slip[static_cast<const DofObject *>(node)][1] =
            slip_direction_two + std::abs(slip_vel_two) * _fe_problem.dt();
    }
    else
      _dof_to_accumulated_slip[static_cast<const DofObject *>(node)] = {0.0, 0.0};
    // End of preparing current accumulated slip

    // Get sign of relative velocity for both directions
    ADReal sign_one = 0.0;
    ADReal sign_two = 0.0;

    if (std::abs(_dof_to_real_tangential_velocity[static_cast<const DofObject *>(node)][0]) >
        TOLERANCE * TOLERANCE)
      sign_one = MathUtils::sign(
          _dof_to_real_tangential_velocity[static_cast<const DofObject *>(node)][0]);

    if (_has_disp_z &&
        std::abs(_dof_to_real_tangential_velocity[static_cast<const DofObject *>(node)][1]) >
            TOLERANCE * TOLERANCE)
      sign_two = MathUtils::sign(
          _dof_to_real_tangential_velocity[static_cast<const DofObject *>(node)][1]);

    // Only accumulate nodal frictional pressure if normal contact pressure is nonzero.
    if (_dof_to_normal_pressure[static_cast<const DofObject *>(node)] > TOLERANCE * TOLERANCE)
    {
      _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][0] =
          sign_one * _penalty_friction *
          MetaPhysicL::raw_value(_dof_to_accumulated_slip[static_cast<const DofObject *>(node)][0]);

      if (_has_disp_z)
        _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][1] =
            sign_two * _penalty_friction *
            MetaPhysicL::raw_value(
                _dof_to_accumulated_slip[static_cast<const DofObject *>(node)][1]);
    }
  }

  // Build a nodal frictional force for consistency (otherwise interpolation breaks Coulomb
  // conditions)
  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    // Only do check if normal pressure is nonzero.
    if (_dof_to_normal_pressure[static_cast<const DofObject *>(node)] > TOLERANCE * TOLERANCE)
    {
      auto & frictional_nodal_pressure_one =
          _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][0];
      auto & frictional_nodal_pressure_two =
          _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][1];

      if (!_has_disp_z &&
          std::abs(frictional_nodal_pressure_one) >
              _friction_coefficient *
                  _dof_to_normal_pressure[static_cast<const DofObject *>(node)] &&
          std::abs(frictional_nodal_pressure_one) > TOLERANCE * TOLERANCE)
      {
        ADReal sign = MathUtils::sign(frictional_nodal_pressure_one);
        frictional_nodal_pressure_one =
            sign * _friction_coefficient *
            _dof_to_normal_pressure[static_cast<const DofObject *>(node)];
      }
      else if (_has_disp_z &&
               std::sqrt(frictional_nodal_pressure_one * frictional_nodal_pressure_one +
                         frictional_nodal_pressure_two * frictional_nodal_pressure_two) >
                   _friction_coefficient *
                       _dof_to_normal_pressure[static_cast<const DofObject *>(node)])
      {
        const auto current_friction_length =
            std::sqrt(frictional_nodal_pressure_one * frictional_nodal_pressure_one +
                      frictional_nodal_pressure_two * frictional_nodal_pressure_two);
        frictional_nodal_pressure_one =
            frictional_nodal_pressure_one / current_friction_length * _friction_coefficient *
            _dof_to_normal_pressure[static_cast<const DofObject *>(node)];
        frictional_nodal_pressure_two =
            frictional_nodal_pressure_two / current_friction_length * _friction_coefficient *
            _dof_to_normal_pressure[static_cast<const DofObject *>(node)];
      }
    }
  }

  // Now that we have consistent nodal frictional values, create an interpolated frictional
  // pressure variable.
  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);
    const auto & frictional_nodal_pressure_one =
        _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][0];
    const auto & frictional_nodal_pressure_two =
        _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][1];
    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(_qrule_msm->n_points()))
    {
      _frictional_contact_force_one[qp] += test_i[qp] * frictional_nodal_pressure_one;
      _frictional_contact_force_two[qp] += test_i[qp] * frictional_nodal_pressure_two;
    }
  }
}

void
PenaltyFrictionUserObject::finalize()
{
  WeightedVelocitiesUserObject::finalize();

  // If the constraint is performed by the owner, then we don't need any data sent back; the owner
  // will take care of it. But if the constraint is not performed by the owner and we might have to
  // do some of the constraining ourselves, then we need data sent back to us
  const bool send_data_back = !constrainedByOwner();

  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_accumulated_slip, _subproblem.mesh(), _nodal, _communicator, send_data_back);
}
