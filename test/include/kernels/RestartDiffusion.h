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
#ifndef RESTARTDIFFUSION_H
#define RESTARTDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class RestartDiffusion;

template <>
InputParameters validParams<RestartDiffusion>();

class RestartDiffusion : public Kernel
{
public:
  RestartDiffusion(const InputParameters & parameters);

  virtual void timestepSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _coef;
  Real & _current_coef;
};

#endif // RESTARTDIFFUSION_H
