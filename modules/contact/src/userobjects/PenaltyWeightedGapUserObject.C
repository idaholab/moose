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
  params.addRangeCheckedParam<Real>(
      "penetration_tolerance",
      1e-9,
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
    _augmented_lagrange_problem(dynamic_cast<AugmentedLagrangianContactProblem *>(&_fe_problem)),
    _lagrangian_iteration_number(_augmented_lagrange_problem
                                     ? _augmented_lagrange_problem->getLagrangianIterationNumber()
                                     : _no_iterations),
    _new_time_step(true)
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

void
PenaltyWeightedGapUserObject::reinit()
{
  _contact_force.resize(_qrule_msm->n_points());
  for (const auto qp : make_range(_qrule_msm->n_points()))
    _contact_force[qp] = 0;

  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);
    const auto & weighted_gap = libmesh_map_find(_dof_to_weighted_gap, node).first;
    const auto lagrange_multiplier =
        _augmented_lagrange_problem ? _dof_to_lagrange_multiplier[node] : 0.0;
    const auto weighted_gap_for_calc = weighted_gap < 0 ? -weighted_gap : ADReal(0);
    const auto & test_i = (*_test)[i];

    const auto normal_pressure = _penalty * weighted_gap_for_calc + lagrange_multiplier;
    _dof_to_normal_pressure[node] = normal_pressure;
    for (const auto qp : make_range(_qrule_msm->n_points()))
      _contact_force[qp] += test_i[qp] * normal_pressure;
  }
}

void
PenaltyWeightedGapUserObject::selfTimestepSetup()
{
  // _dof_to_lagrange_multiplier.clear();
  // _new_time_step = true;
}

void
PenaltyWeightedGapUserObject::timestepSetup()
{
  selfTimestepSetup();
}

bool
PenaltyWeightedGapUserObject::isContactConverged()
{
  std::cout << "Gap: ";
  for (const auto & [dof_object, gap] : _dof_to_weighted_gap)
    std::cout << physicalGap(gap) << ' ';
  std::cout << '\n';

  // // release contact (this could introduce ping pong)
  // bool converged = true;
  // for (const auto & [dof_object, gap] : _dof_to_weighted_gap)
  //   if (gap > _penetration_tolerance && _dof_to_lagrange_multiplier[dof_object] > 0)
  //   {
  //     _dof_to_lagrange_multiplier[dof_object] = 0.0;
  //     converged = false;
  //   }
  // if (!converged)
  //   return false;

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
  std::cout << "old LMs: ";
  for (const auto & [dof_object, lagrange_multiplier] : _dof_to_lagrange_multiplier)
    std::cout << lagrange_multiplier << ' ';
  std::cout << '\n';

  // for (const auto & [dof_object, gap] : _dof_to_weighted_gap)

  for (auto & [dof_object, lagrange_multiplier] : _dof_to_lagrange_multiplier)
  {
    const auto & gap =
        MetaPhysicL::raw_value(libmesh_map_find(_dof_to_weighted_gap, dof_object).first);
    lagrange_multiplier += -gap * _penalty;
    if (lagrange_multiplier < 0.0)
      lagrange_multiplier = 0.0;

    // const auto gap_for_calc = gap.first < 0 ? MetaPhysicL::raw_value(-gap.first) : 0.0;
    // _dof_to_lagrange_multiplier[dof_object] += gap_for_calc * _penalty;
  }

  std::cout << "new LMs: ";
  for (const auto & [dof_object, lagrange_multiplier] : _dof_to_lagrange_multiplier)
    std::cout << lagrange_multiplier << ' ';
  std::cout << '\n';
}
