/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHMATH_H
#define CHMATH_H

#include "CHBulk.h"

// Forward Declarations
class CHMath;

template <>
InputParameters validParams<CHMath>();

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
  CHMath(const InputParameters & parameters);

protected:
  virtual RealGradient computeGradDFDCons(PFFunctionType type);
};

#endif // CHMATH_H
