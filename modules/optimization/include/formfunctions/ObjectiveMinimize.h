#pragma once

#include "FormFunction.h"

class ObjectiveMinimize : public FormFunction
{
public:
  static InputParameters validParams();
  ObjectiveMinimize(const InputParameters & parameters);

  virtual Real computeObjective() override;

private:
  /// vector of simulation data
  std::vector<const PostprocessorValue *> _pp_values;
  /// vector of measured data
  const std::vector<Real> & _measured_values;
};
