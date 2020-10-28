#pragma once

#include "FormFunction.h"

class ObjectiveVppMinimize : public FormFunction
{
public:
  static InputParameters validParams();
  ObjectiveVppMinimize(const InputParameters & parameters);

  virtual Real computeObjective() override;

private:
  /// vector of simulation data
  const VectorPostprocessorValue & _simulation_values;
  /// vector of measured data
  const VectorPostprocessorValue & _measured_values;
};
