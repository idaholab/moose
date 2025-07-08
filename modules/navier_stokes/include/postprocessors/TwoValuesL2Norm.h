#pragma once

#include "ElementIntegralVariablePostprocessor.h"

class TwoValuesL2Norm : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  TwoValuesL2Norm(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  virtual Real computeQpIntegral() override;
  const VariableValue & _other_var;
};
