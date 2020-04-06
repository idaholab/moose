//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSBase.h"

// Forward Declarations

/**
 * This class is responsible for solving the scalar advection
 * equation, possibly with a forcing function.
 */
class Advection : public INSBase
{
public:
  static InputParameters validParams();

  Advection(const InputParameters & parameters);

  virtual ~Advection() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned /*jvar*/) { return 0; }
  const Function & _ffn;
  MooseEnum _tau_type;
};
