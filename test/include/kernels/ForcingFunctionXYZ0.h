//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FORCING_FUNCTION_XYZ0_H
#define FORCING_FUNCTION_XYZ0_H

#include "Kernel.h"
#include "UsrFunc.h"

// Forward Declarations
class ForcingFunctionXYZ0;

template <>
InputParameters validParams<ForcingFunctionXYZ0>();

class ForcingFunctionXYZ0 : public Kernel
{
public:
  ForcingFunctionXYZ0(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

private:
  /**
   *   Parameters for the manufactured solution used.
   */
  Real _A0, _B0, _C0, _Au, _Bu, _Cu, _Av, _Bv, _Cv, _Ak, _Bk, _Ck, _omega0;
};

#endif // FORCING_FUNCTION_XYZ0H
