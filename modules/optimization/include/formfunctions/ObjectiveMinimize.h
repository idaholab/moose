#pragma once

#include "FormFunction.h"

class ObjectiveMinimize : public FormFunction
{
public:
  static InputParameters validParams();
  ObjectiveMinimize(const InputParameters & parameters);

  virtual Real computeObjective() override;

private:
  /// VPP containing the subapp data
  const VectorPostprocessorValue & _subapp_vpp_values;
};
