#pragma once

#include "ElementIntegralVariablePostprocessor.h"

class TempDoppler : public ElementIntegralVariablePostprocessor

{
public:
  static InputParameters validParams();

  TempDoppler(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  virtual Real computeQpIntegral() override;
  /// The variable to compare to
  const VariableValue & _T_ref;
  const VariableValue & _flux;
  const VariableValue & _adjflux;
  Real _a;
  Real _b;
  Real _c;
  Real _d;
};
