#pragma once

#include "InitialCondition.h"

/**
 *
 */
class SpecificVolumeIC : public InitialCondition
{
public:
  SpecificVolumeIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const VariableValue & _rhoA;
  const VariableValue & _area;
  const VariableValue & _alpha;

public:
  static InputParameters validParams();
};
