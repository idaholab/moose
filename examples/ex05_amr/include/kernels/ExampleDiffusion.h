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

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters validParams<ExampleDiffusion>();

/**
 * This Kernel inherits from Diffusion not directly
 * from Kernel.
 */
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
