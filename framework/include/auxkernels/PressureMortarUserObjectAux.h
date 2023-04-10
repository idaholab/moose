//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class UserObject;

/**
 * Function auxiliary value
 */
class PressureMortarUserObjectAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PressureMortarUserObjectAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// What penalty mortar contact quantity we'd like to output
  const MooseEnum _contact_quantity;

  /// PenaltyFrictionUserObject to be queried for a value
  const UserObject * _user_object;

  /// What type of constraint we are going to enforce
  enum ContactQuantity
  {
    ACCUMULATED_SLIP_ONE,
    SLIP_ONE,
    NORMAL_PRESSURE,
    FRICTIONAL_PRESSURE_ONE,
    TANGENTIAL_VELOCITY_ONE
  };
};
