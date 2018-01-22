// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FEFLUXBC_H
#define FEFLUXBC_H

// MOOSE includes
#include "FunctionNeumannBC.h"

// Forward declarations
class FEFluxBC;

template <>
InputParameters validParams<FEFluxBC>();

/// Defines an FE-based BC that strongly encourages the gradients to match
class FEFluxBC : public FunctionNeumannBC
{
public:
  FEFluxBC(const InputParameters & parameters);
};

#endif // FEFLUXBC_H
