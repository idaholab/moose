//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyMortarUserObjectAux.h"

#include "PenaltyFrictionUserObject.h"
#include "PenaltyWeightedGapUserObject.h"

registerMooseObject("ContactApp", PenaltyMortarUserObjectAux);

InputParameters
PenaltyMortarUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Populates an auxiliary variable with a contact quantities from penalty mortar contact.");
  MooseEnum contact_quantity("normal_pressure accumulated_slip_one "
                             "tangential_pressure_one tangential_velocity_one accumulated_slip_two "
                             "tangential_pressure_two tangential_velocity_two weighted_gap");
  params.addRequiredParam<MooseEnum>(
      "contact_quantity",
      contact_quantity,
      "The desired contact quantity to output as an auxiliary variable.");
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The penalty mortar user object to get values from.  Note that the user object "
      "must implement the corresponding getter function.");
  return params;
}

PenaltyMortarUserObjectAux::PenaltyMortarUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters), _contact_quantity(getParam<MooseEnum>("contact_quantity"))
{
  _user_object = &(parameters.get<FEProblemBase *>("_fe_problem_base")
                       ->getUserObjectBase(getParam<UserObjectName>("user_object")));

  if (!isNodal())
    mooseError("This auxiliary kernel requires nodal variables to obtain contact pressure values");

  if (dynamic_cast<const PenaltyWeightedGapUserObject *>(_user_object) &&
      _contact_quantity != ContactQuantity::NORMAL_PRESSURE)
    paramError(
        "contact_quantity",
        "The contact quantity requested is not available for penalty weighted gap user objects.");

  if (!dynamic_cast<const PenaltyWeightedGapUserObject *>(_user_object) &&
      !dynamic_cast<const PenaltyFrictionUserObject *>(_user_object))
    paramError("user_object", "User object type is not supported");
}

Real
PenaltyMortarUserObjectAux::computeValue()
{
  if (_contact_quantity == ContactQuantity::NORMAL_PRESSURE)
  {
    if (dynamic_cast<const PenaltyWeightedGapUserObject *>(_user_object))
      return static_cast<const PenaltyWeightedGapUserObject *>(_user_object)
          ->getNormalContactPressure(_current_node);
    else
      return static_cast<const PenaltyFrictionUserObject *>(_user_object)
          ->getNormalContactPressure(_current_node);
  }
  else if (_contact_quantity == ContactQuantity::WEIGHTED_GAP)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getNormalWeightedGap(_current_node);
  else if (_contact_quantity == ContactQuantity::FRICTIONAL_PRESSURE_ONE)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getFrictionalContactPressure(_current_node, 0);
  else if (_contact_quantity == ContactQuantity::ACCUMULATED_SLIP_ONE)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getAccumulatedSlip(_current_node, 0);
  else if (_contact_quantity == ContactQuantity::TANGENTIAL_VELOCITY_ONE)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getTangentialVelocity(_current_node, 0);
  else if (_contact_quantity == ContactQuantity::FRICTIONAL_PRESSURE_TWO)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getFrictionalContactPressure(_current_node, 1);
  else if (_contact_quantity == ContactQuantity::ACCUMULATED_SLIP_TWO)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getAccumulatedSlip(_current_node, 1);
  else if (_contact_quantity == ContactQuantity::TANGENTIAL_VELOCITY_TWO)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getTangentialVelocity(_current_node, 1);
  else
  {
    mooseError("Internal error: Contact quantity request in PressureMortarUserObjectAux is not "
               "recognized.");
    return 0.0;
  }
}
