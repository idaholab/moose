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
  /// the output of this block (false for _value < _threshold, true otherwise)
  bool & _output;
};

#endif // UNITTRIP_H
