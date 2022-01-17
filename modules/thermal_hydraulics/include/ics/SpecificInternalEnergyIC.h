#pragma once

#include "InitialCondition.h"

/**
 *
 */
class SpecificInternalEnergyIC : public InitialCondition
{
public:
  SpecificInternalEnergyIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const VariableValue & _rho;
  const VariableValue & _rhou;
  const VariableValue & _rhoE;

public:
  static InputParameters validParams();
};
