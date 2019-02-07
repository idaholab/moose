//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LINEARINTERPOLATIONMATERIAL_H
#define LINEARINTERPOLATIONMATERIAL_H

#include "Material.h"

class LinearInterpolationMaterial;
class LinearInterpolation;
class PolynomialFit;

template <>
InputParameters validParams<LinearInterpolationMaterial>();

class LinearInterpolationMaterial : public Material
{
public:
  LinearInterpolationMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  bool _use_poly_fit;
  std::unique_ptr<LinearInterpolation> _linear_interp;
  std::unique_ptr<PolynomialFit> _poly_fit;
  MaterialProperty<Real> & _property;
};

#endif // LINEARINTERPOLATIONMATERIAL_H
