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
#ifndef DEFAULTPOSTPROCESSORDIFFUSION_H
#define DEFAULTPOSTPROCESSORDIFFUSION_H

#include "Kernel.h"

// Forward Declaration
class DefaultPostprocessorDiffusion;

template <>
InputParameters validParams<DefaultPostprocessorDiffusion>();

class DefaultPostprocessorDiffusion : public Kernel
{
public:
  DefaultPostprocessorDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  const PostprocessorValue & _pps_value;
};

#endif // DEFAULTPOSTPROCESSORDIFFUSION_H
