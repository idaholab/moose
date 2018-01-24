//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONNEUMANNBC_H
#define FUNCTIONNEUMANNBC_H

#include "IntegratedBC.h"

// Forward Declarations
class FunctionNeumannBC;
class Function;

template <>
InputParameters validParams<FunctionNeumannBC>();

/**
 * Boundary condition of a Neumann style whose value is computed by a user-defined function
 */
class FunctionNeumannBC : public IntegratedBC
{
public:
  FunctionNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// The function being used for setting the value
  Function & _func;
};

#endif // FUNCTIONNEUMANNBC_H
