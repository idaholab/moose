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

#ifndef COUPLEDTIMEDERIVATIVE_H
#define COUPLEDTIMEDERIVATIVE_H

#include "Kernel.h"

// Forward Declaration
class CoupledTimeDerivative;

template <>
InputParameters validParams<CoupledTimeDerivative>();

/**
 * This calculates the time derivative for a coupled variable
 **/
class CoupledTimeDerivative : public Kernel
{
public:
  CoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const VariableValue & _v_dot;
  const VariableValue & _dv_dot;
  const unsigned int _v_var;
};

#endif // COUPLEDTIMEDERIVATIVE_H
