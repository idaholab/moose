//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodalPostprocessor.h"
#include "MooseVariableInterface.h"

/**
 * This is a base class for other classes which compute post-processed
 * values based on nodal solution values of _u.
 */
class NodalVariablePostprocessor : public NodalPostprocessor, public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  NodalVariablePostprocessor(const InputParameters & parameters);

protected:
  /// Holds the solution at current quadrature points
  const VariableValue & _u;
};
