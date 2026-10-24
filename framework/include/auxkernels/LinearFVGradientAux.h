//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class LinearFVGradientReader;

/**
 * Copies the cell-centered gradient of a linear finite-volume variable, computed with a selected
 * gradient method, into a vector auxiliary variable.
 */
class LinearFVGradientAux : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  LinearFVGradientAux(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeValue() override;

  /// Reader for the gradient of the selected variable and gradient method
  const LinearFVGradientReader & _gradient;
};
