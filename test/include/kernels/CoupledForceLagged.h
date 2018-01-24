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

#ifndef COUPLEDFORCELAGGEDLAGGED_H
#define COUPLEDFORCELAGGEDLAGGED_H

#include "Kernel.h"

// Forward Declaration
class CoupledForceLagged;

template <>
InputParameters validParams<CoupledForceLagged>();

/**
 * CoupledForce using values from previous Newton iterate
 */
class CoupledForceLagged : public Kernel
{
public:
  CoupledForceLagged(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _v_var;
  const VariableValue & _v;
};

#endif // COUPLEDFORCELAGGEDLAGGED_H
