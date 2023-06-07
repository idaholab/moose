//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyWeightedGapUserObject.h"
#include "AugmentedLagrangianContactProblem.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

registerMooseObject("ContactApp", PenaltyWeightedGapUserObject);

const unsigned int PenaltyWeightedGapUserObject::_no_iterations = 0;

InputParameters
PenaltyWeightedGapUserObject::validParams()
{
  InputParameters params = WeightedGapUserObject::validParams();
  params.addClassDescription("Computes the mortar normal contact force via a penalty approach.");
  params.addRequiredParam<Real>("penalty", "The penalty factor");
  params.addRangeCheckedParam<Real>(
      "penalty_multiplier",
      1.0,
      "penalty_multiplier > 0",
      "The penalty growth factor between augmented Lagrange iterations");
  params.addParam<Real>("augmented_lagrange_predictor_scale",
                        0.0,
                        "Perform a linear extrapolation from the last two augmented lagrange "
                        "multipliers to the current timestep");
  params.addRangeCheckedParam<Real>(
      "penetration_tolerance",
      1e-5,
      "penetration_tolerance > 0",
      "Acceptable penetration distance at which augmented Lagrange iterations can be stopped");

  params.addParamNamesToGroup("penalty_multiplier penetration_tolerance", "Augmented Lagrange");

  return params;
}

PenaltyWeightedGapUserObject::PenaltyWeightedGapUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters),
    _penalty(getParam<Real>("penalty")),
    _penalty_multiplier(getParam<Real>("penalty_multiplier")),
    _penetration_tolerance(getParam<Real>("penetration_tolerance")),
    _augmented_lagrange_problem(
        dynamic_cast<AugmentedLagrangianContactProblemInterface *>(&_fe_problem)),
    _lagrangian_iteration_number(_augmented_lagrange_problem
                                     ? _augmented_lagrange_problem->getLagrangianIterationNumber()
                                     : _no_iterations),
    _predictor_scale(getParam<Real>("augmented_lagrange_predictor_scale")),
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
PenaltyWeightedGapUserObject::test() const
{
  return _disp_x_var->phiLower();
}

const ADVariableValue &
PenaltyWeightedGapUserObject::contactPressure() const
{
  return _contact_force;
}

void
PenaltyWeightedGapUserObject::selfInitialize()
{
  _dof_to_normal_pressure.clear();
}

void
PenaltyWeightedGapUserObject::initialize()
{
  WeightedGapUserObject::initialize();
  selfInitialize();
}

Real
PenaltyWeightedGapUserObject::getNormalContactPressure(const Node * const node) const
{
  const auto it = _dof_to_normal_pressure.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_normal_pressure.end())
    return MetaPhysicL::raw_value(it->second);
  else
    return 0.0;
}

Real
PenaltyWeightedGapUserObject::getNormalLagrangeMultiplier(const Node * const node) const
{
  const auto it = _dof_to_lagrange_multiplier.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_lagrange_multiplier.end())
    return MetaPhysicL::raw_value(it->second);
  else
    return 0.0;
}

void
PenaltyWeightedGapUserObject::reinit()
{
  _contact_force.resize(_qrule_msm->n_points());
  for (const auto qp : make_range(_qrule_msm->n_points()))
    _contact_force[qp] = 0;

  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    // Simo's definition of the gap is positive during penetration
    const auto & weighted_gap = -libmesh_map_find(_dof_to_weighted_gap, node).first;
    const auto lagrange_multiplier =
        _augmented_lagrange_problem ? _dof_to_lagrange_multiplier[node] : 0.0;
    const auto & test_i = (*_test)[i];

    // Simo et al. 2.14
    auto normal_pressure = _penalty * weighted_gap + lagrange_multiplier;
    normal_pressure = normal_pressure > 0.0 ? normal_pressure : 0.0;

    _dof_to_normal_pressure[node] = normal_pressure;
    for (const auto qp : make_range(_qrule_msm->n_points()))
      _contact_force[qp] += test_i[qp] * normal_pressure;
  }
}

void
PenaltyWeightedGapUserObject::selfTimestepSetup()
{
  // do not clear the LMs!

  // predict next time step LM linearly
  if (_predictor_scale != 0.0)
    for (auto & [dof_object, lagrange_multiplier] : _dof_to_lagrange_multiplier)
    {
      // save off current LM
      const auto current_lm = lagrange_multiplier;

      // find old LM
      const auto it = _dof_to_old_lagrange_multiplier.find(dof_object);
      if (it != _dof_to_old_lagrange_multiplier.end())
      {
        // Linearly extrapolate LM
        const auto old_lm = it->second;
        lagrange_multiplier += (current_lm - old_lm) / _dt_old * _dt * _predictor_scale;
      }

      // save unupdated LM old LM storage
      _dof_to_old_lagrange_multiplier[dof_object] = current_lm;
    }

  // save old timestep
  _dt_old = _dt;
}

void
PenaltyWeightedGapUserObject::timestepSetup()
{
  selfTimestepSetup();
}

bool
PenaltyWeightedGapUserObject::isContactConverged()
{
  // check if penetration is below threshold
  for (const auto & [dof_object, gap] : _dof_to_weighted_gap)
    if (physicalGap(gap) < -_penetration_tolerance ||
        (physicalGap(gap) > _penetration_tolerance && _dof_to_lagrange_multiplier[dof_object] > 0))
      return false;

  return true;
}

void
PenaltyWeightedGapUserObject::updateAugmentedLagrangianMultipliers()
{
  for (auto & [dof_object, lagrange_multiplier] : _dof_to_lagrange_multiplier)
    if (auto it = _dof_to_weighted_gap.find(dof_object); it != _dof_to_weighted_gap.end())
    {
      // Simo's definition of the gap is positive during penetration
      const auto & gap = -MetaPhysicL::raw_value(it->second.first);

      // Simo et al. 2.15
      lagrange_multiplier += gap * _penalty;
      if (lagrange_multiplier < 0.0)
        lagrange_multiplier = 0.0;
    }
}
