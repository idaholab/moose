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
#ifndef PPSDIFFUSION_H
#define PPSDIFFUSION_H

#include "Kernel.h"

// Forward Declaration
class PPSDiffusion;

template <>
InputParameters validParams<PPSDiffusion>();

class PPSDiffusion : public Kernel
{
public:
  PPSDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  const PostprocessorValue & _pps_value;
};

#endif // PPSDIFFUSION_H
