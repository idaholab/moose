//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADVDIFFREACTION1_H
#define ADVDIFFREACTION1_H

#include "Kernel.h"
#include "UsrFunc.h"

// Forward Declarations
class AdvDiffReaction1;

template <>
InputParameters validParams<AdvDiffReaction1>();

class AdvDiffReaction1 : public Kernel
{
public:
  AdvDiffReaction1(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /**
   *   Parameters for the manufactured solution used.
   */
  Real _A0, _B0, _C0, _Au, _Bu, _Cu, _Av, _Bv, _Cv, _Ak, _Bk, _Ck, _omega0;
};

#endif // ADVDIFFREACTION1_H
