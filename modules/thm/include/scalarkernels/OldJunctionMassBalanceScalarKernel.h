#ifndef OLDJUNCTIONMASSBALANCESCALARKERNEL_H
#define OLDJUNCTIONMASSBALANCESCALARKERNEL_H

#include "JunctionScalarKernel.h"

class OldJunctionMassBalanceScalarKernel;

template <>
InputParameters validParams<OldJunctionMassBalanceScalarKernel>();

/**
 * Constraint to preserve mass balance at a junction
 */
class OldJunctionMassBalanceScalarKernel : public JunctionScalarKernel
{
public:
  OldJunctionMassBalanceScalarKernel(const InputParameters & parameters);

  virtual void computeResidual();
  virtual void computeJacobian();

protected:
  unsigned int _rhoA_var_number;
  unsigned int _rhouA_var_number;
  const VariableValue & _rhouA;
};

#endif /* OLDJUNCTIONMASSBALANCESCALARKERNEL_H */
