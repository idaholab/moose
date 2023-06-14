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

InputParameters
PenaltyMortarUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Populates an auxiliary variable with a contact quantities from penalty mortar contact.");
  MooseEnum contact_quantity("normal_pressure accumulated_slip_one "
                             "tangential_pressure_one tangential_velocity_one accumulated_slip_two "
                             "tangential_pressure_two tangential_velocity_two normal_gap "
                             "normal_lm delta_tangential_lm_one delta_tangential_lm_two");
  params.addRequiredParam<MooseEnum>(
      "contact_quantity",
      contact_quantity,
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
    _user_object(getUserObject<UserObject>("user_object"))
{
  if (!isNodal())
    mooseError("This auxiliary kernel requires nodal variables to obtain contact pressure values");
}

Real
PenaltyMortarUserObjectAux::computeValue()
{
  const auto wguo = dynamic_cast<const WeightedGapUserObject *>(&_user_object);
  const auto pwguo = dynamic_cast<const PenaltyWeightedGapUserObject *>(&_user_object);
  const auto wvuo = dynamic_cast<const WeightedVelocitiesUserObject *>(&_user_object);
  const auto pfuo = dynamic_cast<const PenaltyFrictionUserObject *>(&_user_object);

  switch (_contact_quantity)
  {
    case ContactQuantityEnum::NORMAL_PRESSURE:
      if (pwguo)
        return pwguo->getNormalContactPressure(_current_node);
      else
        paramError("user_object",
                   "The 'normal_pressure' quantity is only provided by a "
                   "'PenaltyWeightedGapUserObject' or derived object.");

    case ContactQuantityEnum::NORMAL_GAP:
      if (wguo)
        return wguo->getNormalGap(_current_node);
      else
        paramError("user_object",
                   "The 'normal_gap' quantity is only provided by a "
                   "'WeightedGapUserObject' or derived object.");

    case ContactQuantityEnum::FRICTIONAL_PRESSURE_ONE:
      if (pfuo)
        return pfuo->getFrictionalContactPressure(_current_node, 0);
      else
        paramError("user_object",
                   "The 'tangential_pressure_one' quantity is only provided by a "
                   "'PenaltyFrictionUserObject' or derived object.");

    case ContactQuantityEnum::ACCUMULATED_SLIP_ONE:
      if (pfuo)
        return pfuo->getAccumulatedSlip(_current_node, 0);
      else
        paramError("user_object",
                   "The 'accumulated_slip_one' quantity is only provided by a "
                   "'PenaltyFrictionUserObject' or derived object.");

    case ContactQuantityEnum::TANGENTIAL_VELOCITY_ONE:
      if (wvuo)
        return wvuo->getTangentialVelocity(_current_node, 0);
      else
        paramError("user_object",
                   "The 'tangential_velocity_one' quantity is only provided by a "
                   "'WeightedVelocitiesUserObject' or derived object.");

    case ContactQuantityEnum::FRICTIONAL_PRESSURE_TWO:
      if (pfuo)
        return pfuo->getFrictionalContactPressure(_current_node, 1);
      else
        paramError("user_object",
                   "The 'tangential_pressure_one' quantity is only provided by a "
                   "'PenaltyFrictionUserObject' or derived object.");

    case ContactQuantityEnum::ACCUMULATED_SLIP_TWO:
      if (pfuo)
        return pfuo->getAccumulatedSlip(_current_node, 1);
      else
        paramError("user_object",
                   "The 'accumulated_slip_two' quantity is only provided by a "
                   "'PenaltyFrictionUserObject' or derived object.");

    case ContactQuantityEnum::TANGENTIAL_VELOCITY_TWO:
      if (wvuo)
        return wvuo->getTangentialVelocity(_current_node, 1);
      else
        paramError("user_object",
                   "The 'tangential_velocity_two' quantity is only provided by a "
                   "'WeightedVelocitiesUserObject' or derived object.");

    case ContactQuantityEnum::NORMAL_LM:
      if (pwguo)
        return pwguo->getNormalLagrangeMultiplier(_current_node);
      else
        paramError("user_object",
                   "The 'normal_lm' quantity is only provided by a "
                   "'PenaltyWeightedGapUserObject' or derived object.");

    case ContactQuantityEnum::DELTA_TANGENTIAL_LM_ONE:
      if (pfuo)
        return pfuo->getDeltaTangentialLagrangeMultiplier(_current_node, 0);
      else
        paramError("user_object",
                   "The 'delta_tangential_lm_one' quantity is only provided by a "
                   "'PenaltyFrictionUserObject' or derived object.");

    case ContactQuantityEnum::DELTA_TANGENTIAL_LM_TWO:
      if (pfuo)
        return pfuo->getDeltaTangentialLagrangeMultiplier(_current_node, 1);
      else
        paramError("user_object",
                   "The 'delta_tangential_lm_two' quantity is only provided by a "
                   "'PenaltyFrictionUserObject' or derived object.");

    default:
      mooseError("Internal error: Contact quantity request in PressureMortarUserObjectAux is not "
                 "recognized.");
  }
}
