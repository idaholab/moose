//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyMortarUserObjectAux.h"

#include "WeightedGapUserObject.h"

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
  : AuxKernel(parameters),
    _contact_quantity(getParam<MooseEnum>("contact_quantity").getEnum<ContactQuantityEnum>())
{
  if (!isNodal())
    mooseError("This auxiliary kernel requires nodal variables to obtain contact pressure values");

  _user_object = dynamic_cast<const WeightedGapUserObject *>(
      &(parameters.get<FEProblemBase *>("_fe_problem_base")
            ->getUserObjectBase(getParam<UserObjectName>("user_object"))));

  if (!_user_object)
    paramError("user_object",
               "The contact quantity requested is not available for the user object requires. Make "
               "sure the penalty mortar contact user object is of the correct type.");
}

Real
PenaltyMortarUserObjectAux::computeValue()
{
  switch (_contact_quantity)
  {
    case ContactQuantityEnum::NORMAL_PRESSURE:
      return _user_object->getNormalContactPressure(_current_node);
      break;
    case ContactQuantityEnum::WEIGHTED_GAP:
      return _user_object->getNormalWeightedGap(_current_node);
      break;
    case ContactQuantityEnum::FRICTIONAL_PRESSURE_ONE:
      return _user_object->getFrictionalContactPressure(_current_node, 0);
      break;
    case ContactQuantityEnum::ACCUMULATED_SLIP_ONE:
      return _user_object->getAccumulatedSlip(_current_node, 0);
      break;
    case ContactQuantityEnum::TANGENTIAL_VELOCITY_ONE:
      return _user_object->getTangentialVelocity(_current_node, 0);
      break;
    case ContactQuantityEnum::FRICTIONAL_PRESSURE_TWO:
      return _user_object->getFrictionalContactPressure(_current_node, 1);
      break;
    case ContactQuantityEnum::ACCUMULATED_SLIP_TWO:
      return _user_object->getAccumulatedSlip(_current_node, 1);
      break;
    case ContactQuantityEnum::TANGENTIAL_VELOCITY_TWO:
      return _user_object->getTangentialVelocity(_current_node, 1);
      break;
    default:
      mooseError("Internal error: Contact quantity request in PressureMortarUserObjectAux is not "
                 "recognized.");
      return 0.0;
  }
}
