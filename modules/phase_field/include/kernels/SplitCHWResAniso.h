/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHWRESANISO_H
#define SPLITCHWRESANISO_H

#include "SplitCHWResBase.h"

/**
 * SplitCHWResAniso creates the residual for the chemical
 * potential in the split form of the Cahn-Hilliard
 * equation with a tensor (anisotropic) mobility.
 */
class SplitCHWResAniso : public SplitCHWResBase<RealTensorValue>
{
public:
  SplitCHWResAniso(const InputParameters & parameters);
};

template <>
InputParameters validParams<SplitCHWResAniso>();

#endif // SPLITCHWRES_H
