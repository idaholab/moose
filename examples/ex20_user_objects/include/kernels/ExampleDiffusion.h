//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Diffusion.h"

class ExampleDiffusion : public Diffusion
{
public:
  ExampleDiffusion(const InputParameters & parameters);

  /**
   * validParams returns the parameters that this Kernel accepts / needs
   * The actual body of the function MUST be in the .C file.
   */
  static InputParameters validParams();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /**
   * This MooseArray will hold the reference we need to our
   * material property from the Material class
   */
  const MaterialProperty<Real> & _diffusivity;
};
