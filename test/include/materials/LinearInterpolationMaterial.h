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
#ifndef LINEARINTERPOLATIONMATERIAL_H
#define LINEARINTERPOLATIONMATERIAL_H

#include "Material.h"

#include "LinearInterpolation.h"
#include "PolynomialFit.h"

class LinearInterpolationMaterial;

template <>
InputParameters validParams<LinearInterpolationMaterial>();

class LinearInterpolationMaterial : public Material
{
public:
  LinearInterpolationMaterial(const InputParameters & parameters);

  virtual ~LinearInterpolationMaterial();

protected:
  virtual void computeQpProperties();

  bool _use_poly_fit;
  LinearInterpolation * _linear_interp;
  PolynomialFit * _poly_fit;
  MaterialProperty<Real> & _property;
};

#endif // LINEARINTERPOLATIONMATERIAL_H
