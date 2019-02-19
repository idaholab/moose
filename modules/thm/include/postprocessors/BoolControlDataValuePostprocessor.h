#ifndef BOOLCONTROLDATAVALUEPOSTPROCESSOR_H
#define BOOLCONTROLDATAVALUEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class BoolControlDataValuePostprocessor;
class Simulation;

template <>
InputParameters validParams<BoolControlDataValuePostprocessor>();

/**
 * Reads a boolean control value data and prints it out
 */
class BoolControlDataValuePostprocessor : public GeneralPostprocessor
{
public:
  BoolControlDataValuePostprocessor(const InputParameters & parameters);

  virtual void initialize();
  virtual Real getValue();
  virtual void execute();

protected:
  Simulation & _sim;
  /// The name of the control data value
  const std::string & _control_data_name;
  /// The boolean value of the control data
  const bool & _control_data_value;
};

#endif /* BOOLCONTROLDATAVALUEPOSTPROCESSOR_H */
