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

#ifndef ONEDEQUALVALUECONSTRAINTBC_H
#define ONEDEQUALVALUECONSTRAINTBC_H

#include "IntegratedBC.h"

class OneDEqualValueConstraintBC;

template <>
InputParameters validParams<OneDEqualValueConstraintBC>();

/**
 * This is the \f$ \int \lambda dg\f$ term from the mortar method.
 * This can connect two 1D domains only.
 *
 * For higher dimensions, you should use face-face constraints.
 */
class OneDEqualValueConstraintBC : public IntegratedBC
{
public:
  OneDEqualValueConstraintBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;

  VariableValue & _lambda;
  unsigned int _lambda_var_number;
  unsigned int _component;
  Real _vg;
};

#endif // ONEDEQUALVALUECONSTRAINTBC_H
