#pragma once

#include "GeneralPostprocessor.h"

class ConstantValuePostprocessor;
class Function;

/**
 * This postprocessor displays a constant value, could be a parameter in a function.
 */
class ConstantValuePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ConstantValuePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  const Real & _value;
};
