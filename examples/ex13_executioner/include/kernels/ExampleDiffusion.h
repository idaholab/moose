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

#ifndef EXAMPLEDIFFUSION_H
#define EXAMPLEDIFFUSION_H

#include "Diffusion.h"

//Forward Declarations
class ExampleDiffusion;

template<>
InputParameters validParams<ExampleDiffusion>();

class ExampleDiffusion : public Diffusion
{
public:

  ExampleDiffusion(const std::string & name,
                   InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  MaterialProperty<Real> & _diffusivity;
};

#endif //EXAMPLEDIFFUSION_H
