//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef CONSERVATIVEADVECTION_H
#define CONSERVATIVEADVECTION_H

#include "Kernel.h"

// Forward Declaration
class ConservativeAdvection;

template <>
InputParameters validParams<ConservativeAdvection>();

class ConservativeAdvection : public Kernel
{
public:
  ConservativeAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  RealVectorValue _velocity;
};

#endif // CONSERVATIVEADVECTION_H
