/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#ifndef SCALARLAGRANGEMULTIPLIER_H
#define SCALARLAGRANGEMULTIPLIER_H

#include "Kernel.h"

class ScalarLagrangeMultiplier;

template <>
InputParameters validParams<ScalarLagrangeMultiplier>();

/**
 * Implements coupling of Lagrange multiplier (as a scalar variable) to a simple constraint of type
 * (g(u) - c = 0)
 */
class ScalarLagrangeMultiplier : public Kernel
{
public:
  ScalarLagrangeMultiplier(const InputParameters & parameters);
  virtual ~ScalarLagrangeMultiplier();

  virtual void computeOffDiagJacobianScalar(unsigned int jvar);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _lambda_var;
  VariableValue & _lambda;
};

#endif /* SCALARLAGRANGEMULTIPLIER_H */
