//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONVECTIONPRECOMPUTE_H
#define CONVECTIONPRECOMPUTE_H

#include "KernelValue.h"

// Forward Declarations
class ConvectionPrecompute;

template <>
InputParameters validParams<ConvectionPrecompute>();

class ConvectionPrecompute : public KernelValue
{
public:
  ConvectionPrecompute(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();

private:
  RealVectorValue _velocity;
};

#endif // CONVECTIONPRECOMPUTE_H
