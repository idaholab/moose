//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"

/**
 * Similar to the ConstantPointSource, but evaluates a Material
 * property at the point source location instead of using a constant
 * value.
 */
class MaterialPointSource : public DiracKernel
{
public:
  static InputParameters validParams();

  MaterialPointSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  const Point & _p;

  const MaterialProperty<Real> * _value;
};
