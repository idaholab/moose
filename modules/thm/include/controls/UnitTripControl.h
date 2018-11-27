#ifndef UNITTRIPCONTROL_H
#define UNITTRIPCONTROL_H

#include "RELAP7Control.h"
#include "MooseParsedFunctionBase.h"

class UnitTripControl;
class RELAP7ParsedFunctionWrapper;

template <>
InputParameters validParams<UnitTripControl>();

/**
 * This control block uses a user-defined condition to determine if a trip happened.
 */
class UnitTripControl : public RELAP7Control, public MooseParsedFunctionBase
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
  std::unique_ptr<RELAP7ParsedFunctionWrapper> _condition_ptr;
};

#endif // UNITTRIPCONTROL_H
