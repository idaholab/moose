#ifndef SCALARLAGRANGEMULTIPLIER_H
#define SCALARLAGRANGEMULTIPLIER_H

#include "Kernel.h"

class ScalarLagrangeMultiplier;

template<>
InputParameters validParams<ScalarLagrangeMultiplier>();

/**
 * Implements coupling of Lagrange multiplier (as a scalar variable) to a simple constraint of type (g(u) - c = 0)
 */
class ScalarLagrangeMultiplier : public Kernel
{
public:
  ScalarLagrangeMultiplier(const std::string & name, InputParameters parameters);
  virtual ~ScalarLagrangeMultiplier();

  virtual void computeOffDiagJacobianScalar(unsigned int jvar);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _lambda_var;
  VariableValue & _lambda;
};

#endif /* SCALARLAGRANGEMULTIPLIER_H */
