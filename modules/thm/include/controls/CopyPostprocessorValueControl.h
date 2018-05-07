#ifndef COPYPOSTPROCESSORVALUECONTROL_H
#define COPYPOSTPROCESSORVALUECONTROL_H

#include "RELAP7Control.h"

class CopyPostprocessorValueControl;

template <>
InputParameters validParams<CopyPostprocessorValueControl>();

/**
 * This control takes a postprocessor and copies its value into a control data value
 */
class CopyPostprocessorValueControl : public RELAP7Control
{
public:
  CopyPostprocessorValueControl(const InputParameters & parameters);

  virtual void execute();

protected:
  Real & _value;
  const Real & _pps_value;
};

#endif // COPYPOSTPROCESSORVALUECONTROL_H
