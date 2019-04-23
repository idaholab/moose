//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declaration
class CoupledForceWrongIndex;

template <>
InputParameters validParams<CoupledForceWrongIndex>();

/**
 * Accesses multiple coupled values but runs out of bounds
 * for testing graceful termination of Moose in this case
 */
class CoupledForceWrongIndex : public Kernel
{
public:
  CoupledForceWrongIndex(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  std::vector<unsigned int> _v_var;
  std::vector<const VariableValue *> _v;
};

