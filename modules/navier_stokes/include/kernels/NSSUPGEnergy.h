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
 * terms in the energy equation.
 */
class NSSUPGEnergy : public NSSUPGBase
{
public:
  static InputParameters validParams();

  NSSUPGEnergy(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  // Single function for computing on and off-diagonal Jacobian
  // entries in a single function.  The input index is in Moose
  // variable numbering.
  Real computeJacobianHelper(unsigned var);
};
