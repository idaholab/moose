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
#ifndef ADVECTION0_H
#define ADVECTION0_H

#include "Kernel.h"
#include "Material.h"

// Forward Declarations
class Advection0;

template <>
InputParameters validParams<Advection0>();

class Advection0 : public Kernel
{
public:
  Advection0(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /// Parameters for spatially linearly varying velocity.
  Real _Au, _Bu, _Cu, _Av, _Bv, _Cv;
};

#endif // ADVECTION0_H
