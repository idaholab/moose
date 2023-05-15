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
#include "ADReal.h"

#include <Eigen/Core>

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

  // save off accumulated slip from the last timestep
  for (auto & map_pr : _dof_to_accumulated_slip)
  {
    auto & [accumulated_slip, old_accumulated_slip] = map_pr.second;
    old_accumulated_slip = accumulated_slip;
  }

  // save off tangential traction from the last timestep
  for (auto & map_pr : _dof_to_tangential_traction)
  {
    auto & [tangential_traction, old_tangential_traction] = map_pr.second;
    old_tangential_traction = {MetaPhysicL::raw_value(tangential_traction(0)),
                               MetaPhysicL::raw_value(tangential_traction(1))};
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
}

Real
PenaltyFrictionUserObject::getFrictionalContactPressure(const Node * const node,
                                                        const unsigned int component) const
{
  const auto it = _dof_to_tangential_traction.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_tangential_traction.end())
    return MetaPhysicL::raw_value(it->second.first(component));
  else
    return 0.0;
}

Real
PenaltyFrictionUserObject::getAccumulatedSlip(const Node * const node,
                                              const unsigned int component) const
{
  const auto it = _dof_to_accumulated_slip.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_accumulated_slip.end())
    return MetaPhysicL::raw_value(it->second.first(component));
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

  // Reset frictional pressure
  _frictional_contact_force_one.resize(_qrule_msm->n_points());
  _frictional_contact_force_two.resize(_qrule_msm->n_points()); // 3D
  for (const auto qp : make_range(_qrule_msm->n_points()))
  {
    _frictional_contact_force_one[qp] = 0.0;
    _frictional_contact_force_two[qp] = 0.0;
  }

  // iterate over nodes
  for (const auto i : make_range(_test->size()))
  {
    // current node
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    // utilized quantities
    const auto & normal_pressure = _dof_to_normal_pressure[node];

    // map the tangential traction and accumulated slip
    auto & [tangential_traction, old_tangential_traction] = _dof_to_tangential_traction[node];
    auto & [accumulated_slip, old_accumulated_slip] = _dof_to_accumulated_slip[node];

    if (normal_pressure > 0.0)
    {
      using namespace std;

      const auto & real_tangential_velocity =
          libmesh_map_find(_dof_to_real_tangential_velocity, node);
      const ADTwoVector slip_velocity = {real_tangential_velocity[0], real_tangential_velocity[1]};

      // tangential trial traction (Simo 3.12)
      ADTwoVector tangential_trial_traction =
          old_tangential_traction + _penalty_friction * slip_velocity * _dt;

      ADReal tangential_trial_traction_norm = tangential_trial_traction.norm();

      ADReal phi_trial = tangential_trial_traction_norm - _friction_coefficient * normal_pressure;

      tangential_traction = tangential_trial_traction;
      if (phi_trial > 0.0)
      {
        // Simo 3.14 (the penalty formulation has an error in the paper)
        ADReal delta_xi = phi_trial; // / _penalty_friction;

        // Simo 3.13
        tangential_traction -=
            delta_xi * tangential_trial_traction / tangential_trial_traction_norm;
      }

      // track accumulated slip for output purposes
      accumulated_slip =
          old_accumulated_slip + MetaPhysicL::raw_value(slip_velocity).cwiseAbs() * _dt;
    }
    else
    {
      // reset slip and clear traction
      accumulated_slip.setZero();
      tangential_traction.setZero();
    }

    // Now that we have consistent nodal frictional values, create an interpolated frictional
    // pressure variable.
    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(_qrule_msm->n_points()))
    {
      _frictional_contact_force_one[qp] += test_i[qp] * tangential_traction(0);
      _frictional_contact_force_two[qp] += test_i[qp] * tangential_traction(1);
    }
  }
}

void
PenaltyFrictionUserObject::finalize()
{
  WeightedVelocitiesUserObject::finalize();
}

bool
PenaltyFrictionUserObject::isContactConverged()
{
  // check normal contact convergence first
  if (!PenaltyWeightedGapUserObject::isContactConverged())
    return false;

  return true;
}

void
PenaltyFrictionUserObject::updateAugmentedLagrangianMultipliers()
{
  PenaltyWeightedGapUserObject::updateAugmentedLagrangianMultipliers();
}
