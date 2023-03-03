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
}

const VariableTestValue &
PenaltyWeightedGapUserObject::test() const
{
  return _disp_x_var->phiLower();
}

bool
PenaltyWeightedGapUserObject::isWeightedGapNodal() const
{
  return _disp_x_var->isNodal();
}

const ADVariableValue &
PenaltyWeightedGapUserObject::contactForce() const
{
  return _contact_force;
}

void
PenaltyWeightedGapUserObject::reinit(const Elem & lower_d_secondary_elem)
{
  mooseAssert(&lower_d_secondary_elem == _lower_secondary_elem,
              "If these match we can simplify some things");
  mooseAssert(lower_d_secondary_elem.n_nodes() == _test->size(), "These must match");

  _contact_force.resize(_qrule_msm->n_points());
  for (const auto qp : make_range(_qrule_msm->n_points()))
    _contact_force[qp] = 0;

  for (const auto i : lower_d_secondary_elem.node_index_range())
  {
    const Node * const node = lower_d_secondary_elem.node_ptr(i);
    const auto & weighted_gap =
        libmesh_map_find(_dof_to_weighted_gap, static_cast<const DofObject *>(node)).first;
    const auto weighted_gap_for_calc = weighted_gap < 0 ? -weighted_gap : ADReal(0);
    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(_qrule_msm->n_points()))
      _contact_force[qp] += (test_i[qp] * _penalty) * weighted_gap_for_calc;
  }
}

bool
PenaltyWeightedGapUserObject::hasDof(const DofObject & dof_object) const
{
  return dof_object.n_dofs(_disp_x_var->sys().number(), _disp_x_var->number());
}
