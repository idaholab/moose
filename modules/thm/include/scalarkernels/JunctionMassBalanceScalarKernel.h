#ifndef JUNCTIONMASSBALANCESCALARKERNEL_H
#define JUNCTIONMASSBALANCESCALARKERNEL_H

#include "JunctionScalarKernel.h"

class JunctionMassBalanceScalarKernel;

template <>
InputParameters validParams<JunctionMassBalanceScalarKernel>();

/**
 * Constraint to preserve mass balance in the junction
 */
class JunctionMassBalanceScalarKernel : public JunctionScalarKernel
{
public:
  JunctionMassBalanceScalarKernel(const InputParameters & parameters);

  virtual void computeResidual();
  virtual void computeJacobian();

protected:
  unsigned int _rhouA_var_number;
  const VariableValue & _rhouA;
};

#endif /* JUNCTIONMASSBALANCESCALARKERNEL_H */
