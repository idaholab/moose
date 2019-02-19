#ifndef ONEDAREATIMESCONSTANTBC_H
#define ONEDAREATIMESCONSTANTBC_H

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

#endif /* ONEDAREATIMESCONSTANTBC_H */
