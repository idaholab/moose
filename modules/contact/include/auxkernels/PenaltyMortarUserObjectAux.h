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
 * Auxiliary kernel to output mortar penalty contact quantities of interest
 */
class PenaltyMortarUserObjectAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PenaltyMortarUserObjectAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// What penalty mortar contact quantity we'd like to output
  const MooseEnum _contact_quantity;

  /// PenaltyFrictionUserObject to be queried for a value
  const UserObject * _user_object;

  /// What type of contact quantity we are querying
  enum ContactQuantity
  {
    NORMAL_PRESSURE,
    ACCUMULATED_SLIP_ONE,
    FRICTIONAL_PRESSURE_ONE,
    TANGENTIAL_VELOCITY_ONE,
    ACCUMULATED_SLIP_TWO,
    FRICTIONAL_PRESSURE_TWO,
    TANGENTIAL_VELOCITY_TWO,
    WEIGHTED_GAP
  };
};
