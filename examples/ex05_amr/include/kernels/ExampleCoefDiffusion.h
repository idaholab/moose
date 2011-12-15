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

#ifndef EXAMPLECOEFDIFFUSION_H
#define EXAMPLECOEFDIFFUSION_H

#include "Kernel.h"

//Forward Declarations
class ExampleCoefDiffusion;

template<>
InputParameters validParams<ExampleCoefDiffusion>();

class ExampleCoefDiffusion : public Kernel
{
public:

  ExampleCoefDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  Real _coef;
};
#endif // EXAMPLECOEFDIFFUSION_H
