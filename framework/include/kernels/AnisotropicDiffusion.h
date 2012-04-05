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

#ifndef ANISOTROPICDIFFUSION_H
#define ANISOTROPICDIFFUSION_H

#include "Kernel.h"

class AnisotropicDiffusion;

template<>
InputParameters validParams<AnisotropicDiffusion>();


class AnisotropicDiffusion : public Kernel
{
public:
  AnisotropicDiffusion(const std::string & name, InputParameters parameters);
  virtual ~AnisotropicDiffusion();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  RealTensorValue _k;
};


#endif /* ANISOTROPICDIFFUSION_H */
