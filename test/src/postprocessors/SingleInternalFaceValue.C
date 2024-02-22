//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SingleInternalFaceValue.h"

registerMooseObject("MooseTestApp", SingleInternalFaceValue);

InputParameters
SingleInternalFaceValue::validParams()
{
  InputParameters params = InternalSidePostprocessor::validParams();

  params.addRequiredCoupledVar("variable", "Variable of interest");
  params.addRequiredParam<dof_id_type>("element_id", "Element of interest");
  params.addRequiredParam<unsigned int>("side_index", "Side of interest");

  MooseEnum state("current old older");
  params.addRequiredParam<MooseEnum>(
      "state", state, "The state for the coupled variable evaluation");
  return params;
}

SingleInternalFaceValue::SingleInternalFaceValue(const InputParameters & parameters)
  : InternalSidePostprocessor(parameters),
    _element_id(getParam<dof_id_type>("element_id")),
    _side_index(getParam<unsigned int>("side_index")),
    _state(getParam<MooseEnum>("state")),
    _value_current(coupledValue("variable")),
    _value_old(coupledValueOld("variable")),
    _value_older(coupledValueOlder("variable")),
    _neighbor_value(coupledNeighborValue("variable")),
    _neighbor_value_old(coupledNeighborValueOld("variable")),
    _neighbor_value_older(coupledNeighborValueOlder("variable"))
{
}

void
SingleInternalFaceValue::execute()
{
  if (_element_id != _current_elem->id() && _side_index != _current_side)
    return;

  if (_state == "current")
    _value = (_value_current[0] + _neighbor_value[0]) / 2;
  else if (_state == "old")
    _value = (_value_old[0] + _neighbor_value_old[0]) / 2;
  else
    _value = (_value_older[0] + _neighbor_value_older[0]) / 2;
}

void
SingleInternalFaceValue::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const SingleInternalFaceValue &>(y);
  _value += pps.getValue();
}

void
SingleInternalFaceValue::finalize()
{
  gatherSum(_value);
}
