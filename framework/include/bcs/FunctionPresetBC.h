//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONPRESETBC_H
#define FUNCTIONPRESETBC_H

#include "PresetNodalBC.h"

// Forward Declarations
class FunctionPresetBC;
class Function;

template <>
InputParameters validParams<FunctionPresetBC>();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
class FunctionPresetBC : public PresetNodalBC
{
public:
  FunctionPresetBC(const InputParameters & parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  virtual Real computeQpValue() override;

  /// Function being used for evaluation of this BC
  Function & _func;
};

#endif // FUNCTIONPRESETBC_H
