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
  params.addParam<bool>("use_mortar_scaled_gap", false, "Whether to use the mortar scaled gap.");
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
    AugmentedLagrangeInterface(this),
    _penalty(getParam<Real>("penalty")),
    _penalty_multiplier(getParam<Real>("penalty_multiplier")),
    _penetration_tolerance(getParam<Real>("penetration_tolerance")),
    _augmented_lagrange_problem(
        dynamic_cast<AugmentedLagrangianContactProblemInterface *>(&_fe_problem)),
    _lagrangian_iteration_number(_augmented_lagrange_problem
                                     ? _augmented_lagrange_problem->getLagrangianIterationNumber()
                                     : _no_iterations),
    _predictor_scale(getParam<Real>("augmented_lagrange_predictor_scale")),
    _dt(_fe_problem.dt()),
    _use_mortar_scaled_gap(getParam<bool>("use_mortar_scaled_gap"))
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
  for (auto & dof_pn : _dof_to_normal_pressure)
    dof_pn.second = 0.0;
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
    return -MetaPhysicL::raw_value(it->second);
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

    const auto penalty =
        findValue(_dof_to_local_penalty, static_cast<const DofObject *>(node), _penalty);

    // Let's keep existing behavior scaling the weighted gap scaled with the element dimension.
    // For AL (and penalty too), it'll be better to just have a physical ('real') gap so that the
    // user (or even our algorithms) can better select this coefficient.
    ADReal gap;
    if (_use_mortar_scaled_gap)
      gap = adPhysicalGap(libmesh_map_find(_dof_to_weighted_gap, node));
    else
      gap = libmesh_map_find(_dof_to_weighted_gap, node).first;

    const auto lagrange_multiplier =
        _augmented_lagrange_problem ? _dof_to_lagrange_multiplier[node] : 0.0;
    const auto & test_i = (*_test)[i];

    auto normal_pressure = penalty * gap + lagrange_multiplier;
    normal_pressure = normal_pressure < 0.0 ? -normal_pressure : 0.0;

    _dof_to_normal_pressure[node] = normal_pressure;
    for (const auto qp : make_range(_qrule_msm->n_points()))
      _contact_force[qp] += test_i[qp] * normal_pressure;
  }
}

void
PenaltyWeightedGapUserObject::selfTimestepSetup()
{
  // clear the LMs
  for (auto & dof_lm : _dof_to_lagrange_multiplier)
    dof_lm.second = 0.0;

  // reset penalty
  for (auto & dof_lp : _dof_to_local_penalty)
    dof_lp.second = _penalty;

  // clear previous gap
  for (auto & dof_pg : _dof_to_previous_gap)
    dof_pg.second = 0.0;

  // save old timestep
  _dt_old = _dt;
}

void
PenaltyWeightedGapUserObject::timestepSetup()
{
  selfTimestepSetup();
}

bool
PenaltyWeightedGapUserObject::isAugmentedLagrangianConverged()
{
  mooseInfoRepeated("PenaltyWeightedGapUserObject::isAugmentedLagrangianConverged()");

  Real max_gap = 0.0;

  // Get maximum gap to ascertain whether we are converged.
  for (auto & [dof_object, wgap] : _dof_to_weighted_gap)
  {
    const auto gap = physicalGap(wgap);
    {
      // check active set nodes
      if (gap < 0)
      {
        if (std::abs(gap) > max_gap)
          max_gap = std::abs(gap);
      }
    }
  }

  if (max_gap > _penetration_tolerance)
  {
    mooseInfoRepeated("Penetration tolerance fail max_gap = ",
                      max_gap,
                      " (gap_tol=",
                      _penetration_tolerance,
                      ")");
    return false;
  }
  else
    mooseInfoRepeated("Penetration tolerance success max_gap = ",
                      max_gap,
                      " (gap_tol=",
                      _penetration_tolerance,
                      ")");

  return true;
}

void
PenaltyWeightedGapUserObject::augmentedLagrangianSetup()
{
  // loop over all nodes for which a gap has been computed
  for (auto & [dof_object, wgap] : _dof_to_weighted_gap)
  {
    const Real gap = physicalGap(wgap);
    // store previous augmented lagrange iteration gap
    _dof_to_previous_gap[dof_object] = gap;
  }
}

void
PenaltyWeightedGapUserObject::updateAugmentedLagrangianMultipliers()
{
  for (const auto & [dof_object, wgap] : _dof_to_weighted_gap)
  {
    auto & penalty = _dof_to_local_penalty[dof_object];
    if (penalty == 0.0)
      penalty = _penalty;

    const auto gap = getNormalGap(static_cast<const Node *>(dof_object));
    auto & lagrange_multiplier = _dof_to_lagrange_multiplier[dof_object];

    // // update lm
    // if (gap < 0)
    //   lagrange_multiplier += gap * penalty;
    // else
    //   lagrange_multiplier = 0.0;

    lagrange_multiplier += std::min(gap * penalty, lagrange_multiplier);

    // update penalty
    const auto previous_gap = _dof_to_previous_gap[dof_object];
    if (gap < 0 && std::abs(gap) > 0.25 * std::abs(previous_gap))
      penalty *= 100.0;
  }
}
