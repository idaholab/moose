/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSSUPGMASS_H
#define NSSUPGMASS_H

#include "NSSUPGBase.h"

// Forward Declarations
class NSSUPGMass;

template <>
InputParameters validParams<NSSUPGMass>();

/**
 * Compute residual and Jacobian terms form the SUPG
 * terms in the mass equation.
 */
class NSSUPGMass : public NSSUPGBase
{
public:
  NSSUPGMass(const InputParameters & parameters);

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

#endif // NSSUPGMASS_H
