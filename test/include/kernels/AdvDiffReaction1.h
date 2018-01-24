/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
