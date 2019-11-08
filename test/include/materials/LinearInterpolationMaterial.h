//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "LinearInterpolation.h"

class PolynomialFit;

class LinearInterpolationMaterial : public Material
{
public:
  static InputParameters validParams();

  LinearInterpolationMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  bool _use_poly_fit;
  std::unique_ptr<LinearInterpolation> _linear_interp;
  std::unique_ptr<PolynomialFit> _poly_fit;
  MaterialProperty<Real> & _property;
};
