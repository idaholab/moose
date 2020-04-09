//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

// Forward Declarations

/**
 * This class implements a form of the Neumann boundary condition in
 * which the boundary term is treated "implicitly".  This concept is
 * discussed by Griffiths, Papanastiou, and others.
 */
class ImplicitNeumannBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  ImplicitNeumannBC(const InputParameters & parameters);

  virtual ~ImplicitNeumannBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};
