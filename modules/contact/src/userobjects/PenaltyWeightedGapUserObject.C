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
      100.0,
      "penalty_multiplier > 0",
      "The penalty growth factor between augmented Lagrange "
      "iterations if weighted gap does not get closed fast enough. For frictional simulations, "
      "values smaller than 100 are recommended, e.g. 5.");
  params.addParam<bool>(
      "use_physical_gap",
      false,
      "Whether to use the physical normal gap (not scaled by mortar integration) and normalize the "
      "penalty coefficient with a representative lower-dimensional volume assigned to the node in "
      "the contacting boundary. This parameter is defaulted to 'true' in the contact action.");
  params.addRangeCheckedParam<Real>(
      "penetration_tolerance",
      1e-5,
      "penetration_tolerance > 0",
      "Acceptable penetration distance at which augmented Lagrange iterations can be stopped");
  params.addCoupledVar(
      "aux_lm",
      "Auxiliary variable that is utilized together with the "
      "penalty approach to interpolate the resulting contact tractions using dual bases.");
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
    _dt(_fe_problem.dt()),
    _use_physical_gap(getParam<bool>("use_physical_gap")),
    _aux_lm_var(isCoupled("aux_lm") ? getVar("aux_lm", 0) : nullptr)
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
  return _aux_lm_var ? _aux_lm_var->phiLower() : _disp_x_var->phiLower();
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

void
PenaltyWeightedGapUserObject::selfFinalize()
{
  // compute new normal pressure for each node
  for (const auto & [dof_object, wgap] : _dof_to_weighted_gap)
  {
    auto penalty = findValue(_dof_to_local_penalty, dof_object, _penalty);

    // If _use_physical_gap is true we "normalize" the penalty parameter with the surrounding area.
    auto gap = _use_physical_gap ? adPhysicalGap(wgap) / wgap.second : wgap.first;

    const auto lagrange_multiplier =
        _augmented_lagrange_problem ? _dof_to_lagrange_multiplier[dof_object] : 0.0;

    // keep the negative normal pressure (compressive per convention here)
    auto normal_pressure = std::min(0.0, penalty * gap + lagrange_multiplier);

    // we switch conventins here and consider positive normal pressure as compressive
    _dof_to_normal_pressure[dof_object] = -normal_pressure;
  }
}

void
PenaltyWeightedGapUserObject::finalize()
{
  WeightedGapUserObject::finalize();
  selfFinalize();
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
    _contact_force[qp] = 0.0;

  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    for (const auto qp : make_range(_qrule_msm->n_points()))
      _contact_force[qp] += (*_test)[i][qp] * _dof_to_normal_pressure[node];
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
  Real max_positive_gap = 0.0;
  Real min_negative_gap = 0.0;

  // Get maximum gap to ascertain whether we are converged.
  for (auto & [dof_object, wgap] : _dof_to_weighted_gap)
  {
    const auto gap = physicalGap(wgap);
    {
      // Check condition for nodes that are active
      if (gap < 0 || _dof_to_lagrange_multiplier[dof_object] < 0.0)
      {
        max_positive_gap = std::max(max_positive_gap, gap);
        min_negative_gap = std::min(min_negative_gap, gap);
      }
    }
  }

  // Communicate the extreme gap values in parallel.
  std::vector<Real> recv;
  if (this->_communicator.rank() == 0)
    recv.resize(this->_communicator.size());
  this->_communicator.gather(
      0, -min_negative_gap > max_positive_gap ? min_negative_gap : max_positive_gap, recv);

  if (this->_communicator.rank() == 0)
  {
    min_negative_gap = *std::min_element(recv.begin(), recv.end());
    max_positive_gap = *std::max_element(recv.begin(), recv.end());

    // report the gap value with the largest magnitude
    const Real reported_gap =
        -min_negative_gap > max_positive_gap ? min_negative_gap : max_positive_gap;
    if (std::abs(reported_gap) > _penetration_tolerance)
    {
      mooseInfoRepeated("Penetration tolerance fail max_gap = ",
                        reported_gap,
                        " (gap_tol=",
                        _penetration_tolerance,
                        "). Iteration number is: ",
                        _lagrangian_iteration_number,
                        ".");
      return false;
    }
    else
      mooseInfoRepeated("Penetration tolerance success max_gap = ",
                        reported_gap,
                        " (gap_tol=",
                        _penetration_tolerance,
                        "). Iteration number is: ",
                        _lagrangian_iteration_number,
                        ".");
  }

  return true;
}

void
PenaltyWeightedGapUserObject::augmentedLagrangianSetup()
{
  // Loop over all nodes for which a gap has been computed
  for (auto & [dof_object, wgap] : _dof_to_weighted_gap)
  {
    const Real gap = physicalGap(wgap);
    // Store previous augmented lagrange iteration gap
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

    const auto possible_normalization =
        (_use_physical_gap ? libmesh_map_find(_dof_to_weighted_gap, dof_object).second : 1.0);

    if (lagrange_multiplier + gap * penalty / possible_normalization <= 0)
      lagrange_multiplier += gap * penalty / possible_normalization;
    else
      lagrange_multiplier = 0.0;

    // Update penalty
    const auto previous_gap = _dof_to_previous_gap[dof_object];
    if (std::abs(gap) > 0.25 * std::abs(previous_gap) && _lagrangian_iteration_number < 6)
      penalty *= _penalty_multiplier;

    // Possible future development: Add possible check for a too-large penalty coefficient.
  }
}
