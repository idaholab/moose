/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAHNHILLIARDANISO_H
#define CAHNHILLIARDANISO_H

#include "CahnHilliardBase.h"

/**
 * SplitCHWRes creates the residual of the Cahn-Hilliard
 * equation with a scalar (isotropic) mobility.
 */
class CahnHilliardAniso : public CahnHilliardBase<RealTensorValue>
{
public:
  CahnHilliardAniso(const InputParameters & parameters);
};

template <>
InputParameters validParams<CahnHilliardAniso>();

#endif // CAHNHILLIARDANISO_H
