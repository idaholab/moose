//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSGRAVITYFORCE_H
#define NSGRAVITYFORCE_H

#include "NSKernel.h"

// Forward Declarations
class NSGravityForce;

template <>
InputParameters validParams<NSGravityForce>();

class NSGravityForce : public NSKernel
{
public:
  NSGravityForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const Real _acceleration;
};

#endif // NSGRAVITYFORCE_H
