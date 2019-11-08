//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "UsrFunc.h"

class ForcingFunctionXYZ0 : public Kernel
{
public:
  static InputParameters validParams();

  ForcingFunctionXYZ0(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

private:
  /**
   *   Parameters for the manufactured solution used.
   */
  Real _A0, _B0, _C0, _Au, _Bu, _Cu, _Av, _Bv, _Cv, _Ak, _Bk, _Ck, _omega0;
};
