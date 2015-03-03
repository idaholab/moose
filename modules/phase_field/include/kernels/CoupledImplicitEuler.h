/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDIMPLICITEULER_H
#define COUPLEDIMPLICITEULER_H

#include "Kernel.h"

// Forward Declaration
class CoupledImplicitEuler;

template<>
InputParameters validParams<CoupledImplicitEuler>();

/**
 * This calculates the time derivative for a coupled variable
 **/
class CoupledImplicitEuler : public Kernel
{
public:
  CoupledImplicitEuler(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  VariableValue & _v_dot;
  VariableValue & _dv_dot;
  unsigned int _v_var;
};

#endif //COUPLEDIMPLICITEULER_H
