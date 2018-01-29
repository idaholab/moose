//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef JACOBIANCHECK_H
#define JACOBIANCHECK_H

#include "NodalKernel.h"

// Forward Declarations
class JacobianCheck;

template <>
InputParameters validParams<JacobianCheck>();

/**
 * Dummy class that tests the Jacobian calculation
 */
class JacobianCheck : public NodalKernel
{
public:
  JacobianCheck(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;
};

#endif /*JACOBIANCHECK_H*/
