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
#include "MooseEnum.h"

class WeightedGapUserObject;

/**
 * Auxiliary kernel to output mortar penalty contact quantities of interest
 */
class PenaltyMortarUserObjectAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the
   * constructor.
   */
  PenaltyMortarUserObjectAux(const InputParameters & parameters);

protected:
  /// What type of contact quantity we are querying
  enum class ContactQuantityEnum
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

  virtual Real computeValue() override;

  /// What penalty mortar contact quantity we'd like to output
  const ContactQuantityEnum _contact_quantity;

  /// The user object inputted by the user to obtain the contact quantities
  const WeightedGapUserObject * _user_object;
};
