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
  params += PenaltyWeightedGapUserObject::validParams();

  params.addClassDescription(
      "Computes the mortar frictional contact force via a penalty approach.");
  params.addParam<Real>("penalty_friction",
                        "The penalty factor for frictional interaction. If not provide, the normal "
                        "penalty factor is also used for the frictional problem.");
  params.addRequiredParam<Real>("friction_coefficient",
                                "The friction coefficient ruling Coulomb friction equations.");
  return params;
}

PenaltyFrictionUserObject::PenaltyFrictionUserObject(const InputParameters & parameters)
  /*
   * We are using vitrual inheritance to avoid teh "Diamond inheritance" problem. This means that
   * that we have to construct WeightedGapUserObject explicitly as it will_not_ be constructed in
   * the intermediate base classes PenaltyWeightedGapUserObject and WeightedVelocitiesUserObject.
   * Virtual inheritance ensures that only one instance of WeightedGapUserObject is included in this
   * class. The inheritance diagram is as follows:
   *
   * WeightedGapUserObject         <-----   PenaltyWeightedGapUserObject
   *          ^                                         ^
   *          |                                         |
   * WeightedVelocitiesUserObject  <-----   PenaltyFrictionUserObject
   *
   */
  : WeightedGapUserObject(parameters),
    PenaltyWeightedGapUserObject(parameters),
    WeightedVelocitiesUserObject(parameters),
    _penalty(getParam<Real>("penalty")),
    _penalty_friction(isParamValid("penalty_friction") ? getParam<Real>("penalty_friction")
                                                       : getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _dt(_fe_problem.dt())
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
  // these functions do not call WeightedGapUserObject::timestepSetup to avoid double initialization
  WeightedVelocitiesUserObject::selfTimestepSetup();
  PenaltyWeightedGapUserObject::selfTimestepSetup();

  // instead we call it explicitly here
  WeightedGapUserObject::timestepSetup();

  _dof_to_old_accumulated_slip.clear();
  _dof_to_old_normal_pressure.clear();

  for (auto & map_pr : _dof_to_normal_pressure)
    _dof_to_old_normal_pressure.emplace(map_pr.first, MetaPhysicL::raw_value(map_pr.second));

  for (auto & [node, slip] : _dof_to_accumulated_slip)
  {
    if (_dof_to_old_normal_pressure[node] >
        TOLERANCE * TOLERANCE) // Only if it had normal pressure. Otherwise, restart count
      _dof_to_old_accumulated_slip[node] = {MetaPhysicL::raw_value(slip[0]),
                                            MetaPhysicL::raw_value(slip[1])};
    else
      _dof_to_old_accumulated_slip[node] = {0.0, 0.0};
  }
}

void
PenaltyFrictionUserObject::initialize()
{
  // these functions do not call WeightedGapUserObject::initialize to avoid double initialization
  WeightedVelocitiesUserObject::selfInitialize();
  PenaltyWeightedGapUserObject::selfInitialize();

  // instead we call it explicitly here
  WeightedGapUserObject::initialize();

  _dof_to_normal_pressure.clear();
  _dof_to_accumulated_slip.clear();
  _dof_to_frictional_pressure.clear();
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
  PenaltyWeightedGapUserObject::reinit();

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
    const auto & normal_pressure = _dof_to_normal_pressure[node];

    const auto & real_tangential_velocity =
        libmesh_map_find(_dof_to_real_tangential_velocity, node);
    const auto & slip_vel_one = real_tangential_velocity[0];
    const auto & slip_vel_two = real_tangential_velocity[1];

    // Get accumulated slip in both directions
    ADReal slip_direction_one = 0.0;
    ADReal slip_direction_two = 0.0;

    if (!_dof_to_old_accumulated_slip.empty())
    {
      slip_direction_one = _dof_to_old_accumulated_slip[node][0];

      if (_has_disp_z)
        slip_direction_two = _dof_to_old_accumulated_slip[node][1];
    }

    // Get current accumulated slip in both directions
    if (normal_pressure > TOLERANCE * TOLERANCE)
    {
      _dof_to_accumulated_slip[node][0] = slip_direction_one + std::abs(slip_vel_one) * _dt;
      if (_has_disp_z)
        _dof_to_accumulated_slip[node][1] = slip_direction_two + std::abs(slip_vel_two) * _dt;
    }
    else
      _dof_to_accumulated_slip[node] = {0.0, 0.0};
    // End of preparing current accumulated slip

    // Get sign of relative velocity for both directions
    ADReal sign_one = 0.0;
    ADReal sign_two = 0.0;

    if (std::abs(_dof_to_real_tangential_velocity[node][0]) > TOLERANCE * TOLERANCE)
      sign_one = MathUtils::sign(_dof_to_real_tangential_velocity[node][0]);

    if (_has_disp_z && std::abs(_dof_to_real_tangential_velocity[node][1]) > TOLERANCE * TOLERANCE)
      sign_two = MathUtils::sign(_dof_to_real_tangential_velocity[node][1]);

    // Only accumulate nodal frictional pressure if normal contact pressure is nonzero.
    if (normal_pressure > TOLERANCE * TOLERANCE)
    {
      _dof_to_frictional_pressure[node][0] =
          sign_one * _penalty_friction * _dof_to_accumulated_slip[node][0];

      if (_has_disp_z)
        _dof_to_frictional_pressure[node][1] =
            sign_two * _penalty_friction * _dof_to_accumulated_slip[node][1];
    }
  }

  // Build a nodal frictional force for consistency (otherwise interpolation breaks Coulomb
  // conditions)
  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);
    const auto & normal_pressure = _dof_to_normal_pressure[node];

    // Only do check if normal pressure is nonzero.
    if (normal_pressure > TOLERANCE * TOLERANCE)
    {
      auto & frictional_nodal_pressure_one = _dof_to_frictional_pressure[node][0];
      auto & frictional_nodal_pressure_two = _dof_to_frictional_pressure[node][1];

      if (!_has_disp_z &&
          std::abs(frictional_nodal_pressure_one) > _friction_coefficient * normal_pressure &&
          std::abs(frictional_nodal_pressure_one) > TOLERANCE * TOLERANCE)
      {
        ADReal sign = MathUtils::sign(frictional_nodal_pressure_one);
        frictional_nodal_pressure_one = sign * _friction_coefficient * normal_pressure;
      }
      else if (_has_disp_z)
      {
        const auto current_friction_length_squared =
            Utility::pow<2>(frictional_nodal_pressure_one) +
            Utility::pow<2>(frictional_nodal_pressure_two);
        if (current_friction_length_squared >
            Utility::pow<2>(_friction_coefficient * normal_pressure))
        {
          const auto current_friction_length = std::sqrt(current_friction_length_squared);
          frictional_nodal_pressure_one = frictional_nodal_pressure_one / current_friction_length *
                                          _friction_coefficient * normal_pressure;
          frictional_nodal_pressure_two = frictional_nodal_pressure_two / current_friction_length *
                                          _friction_coefficient * normal_pressure;
        }
      }
    }
  }

  // Now that we have consistent nodal frictional values, create an interpolated frictional
  // pressure variable.
  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);
    const auto & frictional_nodal_pressure_one = _dof_to_frictional_pressure[node][0];
    const auto & frictional_nodal_pressure_two = _dof_to_frictional_pressure[node][1];
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

bool
PenaltyFrictionUserObject::isContactConverged()
{
  // check normal contact convergence first
  if (!PenaltyWeightedGapUserObject::isContactConverged())
    return false;

  // next check frictional convergence
  std::cout << "Gap: ";
  for (const auto & [dof_object, gap] : _dof_to_weighted_gap)
    std::cout << physicalGap(gap) << ' ';
  std::cout << '\n';

  // check if penetration is below threshold
  for (const auto & [dof_object, gap] : _dof_to_weighted_gap)
    if (physicalGap(gap) < -_penetration_tolerance ||
        (physicalGap(gap) > _penetration_tolerance && _dof_to_lagrange_multiplier[dof_object] > 0))
      return false;

  return true;
}

void
PenaltyFrictionUserObject::updateAugmentedLagrangianMultipliers()
{
  PenaltyWeightedGapUserObject::updateAugmentedLagrangianMultipliers();

  ;

  for (auto & [dof_object, lagrange_multipliers] : _dof_to_frictional_lagrange_multipliers)
  {
    const auto & [flm1, flm2] = lagrange_multipliers;
    // MetaPhysicL::raw_value(libmesh_map_find(_dof_to_weighted_gap, dof_object).first);
    // lagrange_multiplier += -gap * _penalty;
    // if (lagrange_multiplier < 0.0)
    //   lagrange_multiplier = 0.0;

    // const auto gap_for_calc = gap.first < 0 ? MetaPhysicL::raw_value(-gap.first) : 0.0;
    // _dof_to_lagrange_multiplier[dof_object] += gap_for_calc * _penalty;
  }
}
