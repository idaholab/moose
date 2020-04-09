//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CHBulk.h"

/**Cahn-Hilliard Kernel implementing the free energy f = 1/4(1-c^2)^2, such that grad df/dc = (3 c^2
 *-1) grad_c.
 * Most of the Cahn-Hilliard equation is implemented in CHBulk and CHInterface.  This kernel
 *inherents from CHBulk
 * and implements a simple polynomial double well to model spinodal decomposition.
 * See M.R. Tonks et al. / Computational Materials Science 51 (2012) 20-29, Eqs 11 and 12.
 **/
class CHMath : public CHBulk<Real>
{
public:
  static InputParameters validParams();

  CHMath(const InputParameters & parameters);

protected:
  virtual RealGradient computeGradDFDCons(PFFunctionType type);
};
