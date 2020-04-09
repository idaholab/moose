//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSSUPGBase.h"

// Forward Declarations

/**
 * Compute residual and Jacobian terms form the SUPG
 * terms in the momentum equation.
 */
class NSSUPGMomentum : public NSSUPGBase
{
public:
  static InputParameters validParams();

  NSSUPGMomentum(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // This kernel is to be used for the x, y, and z momentum equations.
  // The _component parameter tells you which equation you are currently
  // solving.
  unsigned int _component;

private:
  // Single function for computing on and off-diagonal Jacobian
  // entries in a single function.  The input index is in Moose
  // variable numbering.
  Real computeJacobianHelper(unsigned int var);
};
