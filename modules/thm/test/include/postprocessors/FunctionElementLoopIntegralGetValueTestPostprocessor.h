#pragma once

#include "GeneralPostprocessor.h"

class FunctionElementLoopIntegralUserObject;

/**
 * Gets the value from a FunctionElementLoopIntegralUserObject.
 */
class FunctionElementLoopIntegralGetValueTestPostprocessor : public GeneralPostprocessor
{
public:
  FunctionElementLoopIntegralGetValueTestPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// User object to get value from
  const FunctionElementLoopIntegralUserObject & _uo;

public:
  static InputParameters validParams();
};
