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

template<>
InputParameters validParams<OptionallyCoupledForce>();

/**
 * Simple class to demonstrate off diagonal Jacobian contributions.
 */
class OptionallyCoupledForce : public Kernel
{
public:
  OptionallyCoupledForce(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _v_var;
  VariableValue & _v;
};

#endif //OPTIONALLYCOUPLEDFORCE_H
