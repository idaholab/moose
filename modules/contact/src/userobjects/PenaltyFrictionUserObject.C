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

registerMooseObject("ContactApp", PenaltyFrictionUserObject);

InputParameters
PenaltyFrictionUserObject::validParams()
{
  InputParameters params = WeightedVelocitiesUserObject::validParams();
  params.addRequiredParam<Real>("penalty", "The penalty factor for normal interaction");
  params.addParam<Real>("penalty_friction", 0.0, "The penalty factor for frictional interaction");
  params.addRequiredParam<Real>("friction_coefficient",
                                "The friction coefficient ruling Coulomb friction equations.");
  return params;
}

PenaltyFrictionUserObject::PenaltyFrictionUserObject(const InputParameters & parameters)
  : WeightedVelocitiesUserObject(parameters),
    _penalty(getParam<Real>("penalty")),
    _penalty_friction(isParamSetByUser("penalty_friction") ? getParam<Real>("penalty_friction")
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
    _dof_to_old_normal_pressure.emplace(map_pr);

  for (auto & map_pr : _dof_to_accumulated_slip)
  {
    if (_dof_to_old_normal_pressure[map_pr.first] >
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
  for (auto & map_pr : _dof_to_normal_pressure)
    if (map_pr.first->id() == node->id())
      return libmesh_map_find(_dof_to_normal_pressure,
                              static_cast<const DofObject *>(map_pr.first));
  return 0.0;
}

Real
PenaltyFrictionUserObject::getFrictionalContactPressure(const Node * const node,
                                                        const unsigned int /*component*/) const
{
  for (auto & map_pr : _dof_to_frictional_pressure)
    if (map_pr.first->id() == node->id())
      return libmesh_map_find(_dof_to_frictional_pressure,
                              static_cast<const DofObject *>(map_pr.first))[0];
  return 0.0;
}

Real
PenaltyFrictionUserObject::getAccumulatedSlip(const Node * const node,
                                              const unsigned int /*component*/) const
{
  for (auto & map_pr : _dof_to_accumulated_slip)
    if (map_pr.first->id() == node->id())
    {
      const auto & slip_x = libmesh_map_find(_dof_to_accumulated_slip,
                                             static_cast<const DofObject *>(map_pr.first))[0];
      return MetaPhysicL::raw_value(slip_x);
    }
  return 0.0;
}

Real
PenaltyFrictionUserObject::getTangentialVelocity(const Node * const node,
                                                 const unsigned int /*component*/) const
{
  for (auto & map_pr : _dof_to_real_tangential_velocity)
    if (map_pr.first->id() == node->id())
    {
      const auto & tan_vel_x = libmesh_map_find(_dof_to_real_tangential_velocity,
                                                static_cast<const DofObject *>(map_pr.first))[0];
      return MetaPhysicL::raw_value(tan_vel_x);
    }
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
          MetaPhysicL::raw_value(_penalty * weighted_gap_for_calc);
    }
  }

  // Frictional pressure (Assume 2D)
  _frictional_contact_force_one.resize(_qrule_msm->n_points());
  for (const auto qp : make_range(_qrule_msm->n_points()))
    _frictional_contact_force_one[qp] = 0;

  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);
    const auto & slip_vel_one =
        libmesh_map_find(_dof_to_real_tangential_velocity, static_cast<const DofObject *>(node))[0];

    // Check we are not in an initial simulation step
    Real slip_direction_one(0.0);

    if (_dof_to_old_accumulated_slip.size() > 0)
      slip_direction_one = MetaPhysicL::raw_value(
          _dof_to_old_accumulated_slip[static_cast<const DofObject *>(node)][0]);

    if (_dof_to_normal_pressure[static_cast<const DofObject *>(node)] > TOLERANCE * TOLERANCE)
      _dof_to_accumulated_slip[static_cast<const DofObject *>(node)][0] =
          slip_direction_one + std::abs(slip_vel_one) * _fe_problem.dt();
    else
      _dof_to_accumulated_slip[static_cast<const DofObject *>(node)][0] = 0.0;
    // End of preparing current accumulated slip

    // Get sign of relative velocity
    Real sign = 0.0;
    if (std::abs(_dof_to_real_tangential_velocity[static_cast<const DofObject *>(node)][0]) >
        TOLERANCE * TOLERANCE)
      sign = MetaPhysicL::raw_value(
          (_dof_to_real_tangential_velocity[static_cast<const DofObject *>(node)][0]) /
          std::abs(_dof_to_real_tangential_velocity[static_cast<const DofObject *>(node)][0]));

    // Only accumulate if normal contact force is nonzero.
    if (_dof_to_normal_pressure[static_cast<const DofObject *>(node)] > TOLERANCE * TOLERANCE)
      _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][0] =
          sign * _penalty_friction *
          MetaPhysicL::raw_value(_dof_to_accumulated_slip[static_cast<const DofObject *>(node)][0]);
  }

  // Build a nodal frictional force for consistency (otherwise interpolation breaks Coulomb
  // conditions)
  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);
    auto & frictional_nodal_pressure =
        _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][0];
    if (std::abs(frictional_nodal_pressure) >
            _friction_coefficient * _dof_to_normal_pressure[static_cast<const DofObject *>(node)] &&
        std::abs(frictional_nodal_pressure) > TOLERANCE * TOLERANCE)
    {
      Real sign = 0.0;
      sign =
          MetaPhysicL::raw_value(std::abs(frictional_nodal_pressure) / frictional_nodal_pressure);
      frictional_nodal_pressure = sign * _friction_coefficient *
                                  _dof_to_normal_pressure[static_cast<const DofObject *>(node)];
    }
  }

  // Now that we have consistent nodal frictional values, create an interpolated frictional
  // pressure variable.
  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);
    const auto & frictional_nodal_pressure =
        _dof_to_frictional_pressure[static_cast<const DofObject *>(node)][0];
    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(_qrule_msm->n_points()))
      _frictional_contact_force_one[qp] += test_i[qp] * frictional_nodal_pressure;
  }
}
