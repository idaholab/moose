//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PressureMortarUserObjectAux.h"
#include "PenaltyFrictionUserObject.h"

registerMooseObject("MooseApp", PressureMortarUserObjectAux);

InputParameters
PressureMortarUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Populates an auxiliary variable with a spatial value returned from a "
                             "UserObject spatialValue method.");
  MooseEnum contact_quantity("accumulated_slip_one slip_one normal_pressure "
                             "tangential_pressure_one tangential_velocity_one");
  params.addRequiredParam<MooseEnum>(
      "contact_quantity",
      contact_quantity,
      "The desired contact quantity to output as an auxiliary variable.");
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The UserObject UserObject to get values from.  Note that the UserObject "
      "_must_ implement the spatialValue() virtual function!");
  return params;
}

PressureMortarUserObjectAux::PressureMortarUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters), _contact_quantity(getParam<MooseEnum>("contact_quantity"))
{
  _user_object = &(parameters.get<FEProblemBase *>("_fe_problem_base")
                       ->getUserObjectBase(getParam<UserObjectName>("user_object")));

  if (!isNodal())
    mooseError("This auxiliary kernel requires nodal variables to obtain contact pressure values");
}

Real
PressureMortarUserObjectAux::computeValue()
{
  if (_contact_quantity == ContactQuantity::NORMAL_PRESSURE)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getNormalContactPressure(_current_node);
  else if (_contact_quantity == ContactQuantity::FRICTIONAL_PRESSURE_ONE)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getFrictionalContactPressure(_current_node, 0);
  else if (_contact_quantity == ContactQuantity::ACCUMULATED_SLIP_ONE)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getAccumulatedSlip(_current_node, 0);
  else if (_contact_quantity == ContactQuantity::TANGENTIAL_VELOCITY_ONE)
    return static_cast<const PenaltyFrictionUserObject *>(_user_object)
        ->getTangentialVelocity(_current_node, 0);
  else
  {
    mooseError("Internal error: Contact quantity request in PressureMortarUserObjectAux is not "
               "recognized.");
    return 0.0;
  }
}
