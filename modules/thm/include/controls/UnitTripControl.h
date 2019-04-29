#pragma once

#include "THMControl.h"
#include "MooseParsedFunctionBase.h"

class UnitTripControl;
class THMParsedFunctionWrapper;

template <>
InputParameters validParams<UnitTripControl>();

/**
 * This control block uses a user-defined condition to determine if a trip happened.
 */
class UnitTripControl : public THMControl, public MooseParsedFunctionBase
{
public:
  UnitTripControl(const InputParameters & parameters);

  virtual void init() override;
  virtual void execute() override;

protected:
  /// The user-defined condition
  std::string _condition;
  /// The state of this control object
  bool & _state;
  /// Determines if the state of the trip should stay true for the rest of the simulation after the trip happened
  const bool & _latch;
  /// true if the trip happened, otherwise false
  bool _tripped;

  /// Pointer to the Parsed function wrapper object
  std::unique_ptr<THMParsedFunctionWrapper> _condition_ptr;
};
