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

#ifndef OPTIONALLYCOUPLEDFORCE_H
#define OPTIONALLYCOUPLEDFORCE_H

#include "Kernel.h"

// Forward Declaration
class OptionallyCoupledForce;

template <>
InputParameters validParams<OptionallyCoupledForce>();

/**
 * Simple class to demonstrate off diagonal Jacobian contributions.
 */
class OptionallyCoupledForce : public Kernel
{
public:
  OptionallyCoupledForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _v_var;
  const VariableValue & _v;
  const VariableGradient & _grad_v;
  const VariableSecond & _second_v;
  const VariableValue & _v_dot;
  const VariableValue & _v_dot_du;
  bool _v_coupled;
};

#endif // OPTIONALLYCOUPLEDFORCE_H
