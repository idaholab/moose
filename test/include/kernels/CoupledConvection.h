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
#ifndef COUPLEDCONVECTION_H_
#define COUPLEDCONVECTION_H_

#include "Kernel.h"

class CoupledConvection;

template <>
InputParameters validParams<CoupledConvection>();

/**
 * Define the Kernel for a convection operator that looks like:
 *
 * grad_some_var dot u'
 *
 */
class CoupledConvection : public Kernel
{
public:
  CoupledConvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  const VariableGradient & _velocity_vector;
};

#endif // COUPLEDCONVECTION_H
