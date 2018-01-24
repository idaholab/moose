//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
