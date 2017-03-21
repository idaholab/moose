/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAHNHILLIARD_H
#define CAHNHILLIARD_H

#include "CahnHilliardBase.h"

/**
 * SplitCHWRes creates the residual of the Cahn-Hilliard
 * equation with a scalar (isotropic) mobility.
 */
class CahnHilliard : public CahnHilliardBase<Real>
{
public:
  CahnHilliard(const InputParameters & parameters);
};

template <>
InputParameters validParams<CahnHilliard>();

#endif // CAHNHILLIARD_H
