//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 *  Computes the Gauss's law Laplacian operator (including the electrical
 *  conductivity represented as a material property) acting on the electrostatic
 *  potential
 */
class ConductivityLaplacian : public ADKernel
{
public:
  static InputParameters validParams();

  ConductivityLaplacian(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Electrical conductivity of the material
  const ADMaterialProperty<Real> & _conductivity;
};
