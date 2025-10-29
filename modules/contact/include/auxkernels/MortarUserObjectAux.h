//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "MooseEnum.h"

#include <functional>
#include <map>
#include <tuple>
#include <string>

class UserObject;
class WeightedGapUserObject;
class PenaltyWeightedGapUserObject;
class WeightedVelocitiesUserObject;
class PenaltyFrictionUserObject;

/**
 * Auxiliary kernel to output mortar penalty contact quantities of interest
 */
class MortarUserObjectAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the
   * constructor.
   */
  MortarUserObjectAux(const InputParameters & parameters);

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
    NORMAL_GAP,
    NORMAL_LM,
    DELTA_TANGENTIAL_LM_ONE,
    DELTA_TANGENTIAL_LM_TWO,
    ACTIVE_SET
  };

  virtual Real computeValue() override;

  /// What penalty mortar contact quantity we'd like to output
  const ContactQuantityEnum _contact_quantity;

  /// The user object inputted by the user to obtain the contact quantities
  const UserObject & _user_object;

  ///@{ Cast pointers to specific UOs
  const WeightedGapUserObject * _wguo;
  const PenaltyWeightedGapUserObject * _pwguo;
  const WeightedVelocitiesUserObject * _wvuo;
  const PenaltyFrictionUserObject * _pfuo;
  ///@}

  /// Definition of the output quantities and
  std::map<ContactQuantityEnum, std::tuple<std::string, const void *, std::function<Real(void)>>>
      _outputs;

  /// if true computeValue only performs error checking (used in constructor)
  bool _check_only;

  /// available contact quantities
  static const MooseEnum _contact_quantities;
};
