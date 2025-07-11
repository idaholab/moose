#pragma once

#include "ElementIntegralVariablePostprocessor.h"

class TempDensity : public ElementIntegralVariablePostprocessor

{
public:
  static InputParameters validParams();

  TempDensity(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  virtual Real computeQpIntegral() override;
  /// The variable to compare to
  const VariableValue & _T_ref;
  const VariableValue & _flux;
  Real _a;
  Real _b;
};
