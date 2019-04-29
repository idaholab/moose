#pragma once

#include "OneDNodalBC.h"

class OneDAreaTimesConstantBC;

template <>
InputParameters validParams<OneDAreaTimesConstantBC>();

/**
 *
 */
class OneDAreaTimesConstantBC : public OneDNodalBC
{
public:
  OneDAreaTimesConstantBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const Real & _value;
  const VariableValue & _area;
};
