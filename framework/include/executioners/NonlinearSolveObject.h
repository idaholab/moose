//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

class NonlinearSystemBase;

/**
 * A solve object for use with a nonlinear system solver
 */
class NonlinearSolveObject : public SolveObject
{
public:
  NonlinearSolveObject(Executioner & ex);

  static InputParameters validParams();

protected:
  /// Reference to nonlinear system base for faster access
  NonlinearSystemBase & _nl;
};
