#ifndef REALCONTROLDATAVALUEPOSTPROCESSOR_H
#define REALCONTROLDATAVALUEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class RealControlDataValuePostprocessor;
class Simulation;

template <>
InputParameters validParams<RealControlDataValuePostprocessor>();

/**
 * Reads a control value data and prints it out
 */
class RealControlDataValuePostprocessor : public GeneralPostprocessor
{
public:
  RealControlDataValuePostprocessor(const InputParameters & parameters);

  virtual void initialize();
  virtual Real getValue();
  virtual void execute();

protected:
  Simulation & _sim;
  /// The name of the control data value
  const std::string & _control_data_name;
  /// The value of the control data
  const Real & _control_data_value;
};

#endif /* REALCONTROLDATAVALUEPOSTPROCESSOR_H */
