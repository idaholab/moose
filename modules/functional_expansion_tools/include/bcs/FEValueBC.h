// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FEVALUEBC_H
#define FEVALUEBC_H

// MOOSE includes
#include "FunctionDirichletBC.h"

// Forward declarations
class FEValueBC;

template <>
InputParameters validParams<FEValueBC>();

/**
 * Defines an FE-based boundary condition that forces the values to match
 */
class FEValueBC : public FunctionDirichletBC
{
public:
  FEValueBC(const InputParameters & parameters);
};

#endif // FEVALUEBC_H
