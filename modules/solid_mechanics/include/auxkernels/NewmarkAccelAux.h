//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 *Computes Acceleration using Newmark Time integration scheme
 */
class NewmarkAccelAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NewmarkAccelAux(const InputParameters & parameters);

  virtual ~NewmarkAccelAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _disp_old;
  const VariableValue & _disp;
  const VariableValue & _vel_old;
  const VariableValue & _u_old;
  Real _beta;
};
