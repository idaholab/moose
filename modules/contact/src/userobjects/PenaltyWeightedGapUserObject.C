//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyWeightedGapUserObject.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

registerMooseObject("ContactApp", PenaltyWeightedGapUserObject);

InputParameters
PenaltyWeightedGapUserObject::validParams()
{
  InputParameters params = WeightedGapUserObject::validParams();
  params.addRequiredParam<Real>("penalty", "The penalty factor");
  return params;
}

PenaltyWeightedGapUserObject::PenaltyWeightedGapUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters), _penalty(getParam<Real>("penalty"))
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
PenaltyWeightedGapUserObject::reinit()
{
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
      _contact_force[qp] += (test_i[qp] * _penalty) * weighted_gap_for_calc;
  }
}
