//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
