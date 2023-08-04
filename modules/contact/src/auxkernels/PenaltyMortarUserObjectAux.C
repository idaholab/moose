//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyMortarUserObjectAux.h"

// supported user objects
#include "WeightedGapUserObject.h"
#include "PenaltyWeightedGapUserObject.h"
#include "WeightedVelocitiesUserObject.h"
#include "PenaltyFrictionUserObject.h"

registerMooseObject("ContactApp", PenaltyMortarUserObjectAux);

const MooseEnum PenaltyMortarUserObjectAux::_contact_quantities(
    "normal_pressure accumulated_slip_one "
    "tangential_pressure_one tangential_velocity_one accumulated_slip_two "
    "tangential_pressure_two tangential_velocity_two normal_gap "
    "normal_lm delta_tangential_lm_one delta_tangential_lm_two active_set");

InputParameters
PenaltyMortarUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Populates an auxiliary variable with a contact quantities from penalty mortar contact.");
  params.addRequiredParam<MooseEnum>(
      "contact_quantity",
      _contact_quantities,
      "The desired contact quantity to output as an auxiliary variable.");
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The penalty mortar user object to get values from.  Note that the user object "
      "must implement the corresponding getter function.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

PenaltyMortarUserObjectAux::PenaltyMortarUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _contact_quantity(getParam<MooseEnum>("contact_quantity").getEnum<ContactQuantityEnum>()),
    _user_object(getUserObject<UserObject>("user_object")),
    _wguo(dynamic_cast<const WeightedGapUserObject *>(&_user_object)),
    _pwguo(dynamic_cast<const PenaltyWeightedGapUserObject *>(&_user_object)),
    _wvuo(dynamic_cast<const WeightedVelocitiesUserObject *>(&_user_object)),
    _pfuo(dynamic_cast<const PenaltyFrictionUserObject *>(&_user_object)),
    _outputs({
        {ContactQuantityEnum::NORMAL_PRESSURE,
         {"PenaltyWeightedGapUserObject",
          _pwguo,
          [&]() { return _pwguo->getNormalContactPressure(_current_node); }}},

        {ContactQuantityEnum::NORMAL_GAP,
         {"WeightedGapUserObject", _wguo, [&]() { return _wguo->getNormalGap(_current_node); }}},

        {ContactQuantityEnum::FRICTIONAL_PRESSURE_ONE,
         {"PenaltyFrictionUserObject",
          _pfuo,
          [&]() { return _pfuo->getFrictionalContactPressure(_current_node, 0); }}},

        {ContactQuantityEnum::ACCUMULATED_SLIP_ONE,
         {"PenaltyFrictionUserObject",
          _pfuo,
          [&]() { return _pfuo->getAccumulatedSlip(_current_node, 0); }}},

        {ContactQuantityEnum::TANGENTIAL_VELOCITY_ONE,
         {"WeightedVelocitiesUserObject",
          _wvuo,
          [&]() { return _wvuo->getTangentialVelocity(_current_node, 0); }}},

        {ContactQuantityEnum::FRICTIONAL_PRESSURE_TWO,
         {"PenaltyFrictionUserObject",
          _pfuo,
          [&]() { return _pfuo->getFrictionalContactPressure(_current_node, 1); }}},

        {ContactQuantityEnum::ACCUMULATED_SLIP_TWO,
         {"PenaltyFrictionUserObject",
          _pfuo,
          [&]() { return _pfuo->getAccumulatedSlip(_current_node, 1); }}},

        {ContactQuantityEnum::TANGENTIAL_VELOCITY_TWO,
         {"WeightedVelocitiesUserObject",
          _wvuo,
          [&]() { return _wvuo->getTangentialVelocity(_current_node, 1); }}},

        {ContactQuantityEnum::NORMAL_LM,
         {"PenaltyWeightedGapUserObject",
          _pwguo,
          [&]() { return _pwguo->getNormalLagrangeMultiplier(_current_node); }}},

        {ContactQuantityEnum::DELTA_TANGENTIAL_LM_ONE,
         {"PenaltyFrictionUserObject",
          _wvuo,
          [&]() { return _pfuo->getDeltaTangentialLagrangeMultiplier(_current_node, 0); }}},

        {ContactQuantityEnum::DELTA_TANGENTIAL_LM_TWO,
         {"PenaltyFrictionUserObject",
          _wvuo,
          [&]() { return _pfuo->getDeltaTangentialLagrangeMultiplier(_current_node, 1); }}},

        {ContactQuantityEnum::ACTIVE_SET,
         {"PenaltyWeightedGapUserObject",
          _wvuo,
          [&]() { return _pwguo->getActiveSetState(_current_node) ? 1.0 : 0.0; }}}
        // end outputs list
    })
{
  if (!isNodal())
    mooseError("This auxiliary kernel requires nodal variables to obtain contact pressure values");

  // error check
  const auto it = _outputs.find(_contact_quantity);
  if (it == _outputs.end())
    mooseError("Internal error: Contact quantity request in PressureMortarUserObjectAux is not "
               "recognized.");
  if (!std::get<1>(it->second))
    paramError("user_object",
               "The '",
               _contact_quantities.getNames()[static_cast<int>(it->first)],
               "' quantity is only provided by a '",
               std::get<0>(it->second),
               "' or derived object.");
}

Real
PenaltyMortarUserObjectAux::computeValue()
{
  // execute functional to retrieve selected quantity
  return std::get<2>(_outputs[_contact_quantity])();
}
