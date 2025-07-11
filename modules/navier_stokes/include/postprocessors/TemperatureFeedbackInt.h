#pragma once

#include "ElementIntegralVariablePostprocessor.h"

class TemperatureFeedbackInt : public ElementIntegralVariablePostprocessor

{
public:
  static InputParameters validParams();

  TemperatureFeedbackInt(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  virtual Real computeQpIntegral() override;
  /// The variable to compare to
  const VariableValue & _T_ref;
  const VariableValue & _flux;
  const PostprocessorValue & _Norm;
  Real _total_rho;
};
