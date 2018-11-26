#ifndef UNITTRIP_H
#define UNITTRIP_H

#include "RELAP7Control.h"

class UnitTripControl;

template <>
InputParameters validParams<UnitTripControl>();

/**
 * This control block reads a value as an input, compares it to a user specified threshold and if
 * the value is smaller it produces false, otherwise true.
 */
class UnitTripControl : public RELAP7Control
{
public:
  UnitTripControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// input data that we compare against a threshold
  const Real & _value;
  /// threshold that trips the output from false to true
  const Real & _threshold;
  /// the state of this block (false for _value < _threshold, true otherwise)
  bool & _state;
  /// Determines if the state of the trip should stay true for the rest of the simulation after the trip happened
  const bool & _latch;
  /// true if the trip happened, otherwise false
  bool _tripped;
};

#endif // UNITTRIP_H
