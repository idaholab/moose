/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHWRES_H
#define SPLITCHWRES_H

#include "SplitCHWResBase.h"

/**
 * SplitCHWRes creates the residual for the chemical
 * potential in the split form of the Cahn-Hilliard
 * equation with a scalar (isotropic) mobility.
 */
class SplitCHWRes : public SplitCHWResBase<Real>
{
public:
  SplitCHWRes(const InputParameters & parameters);
};

template <>
InputParameters validParams<SplitCHWRes>();

#endif // SPLITCHWRES_H
