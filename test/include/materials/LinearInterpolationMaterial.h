#ifndef LINEARINTERPOLATIONMATERIAL_H
#define LINEARINTERPOLATIONMATERIAL_H

#include "Material.h"

#include "LinearInterpolation.h"
#include "PolynomialFit.h"

class LinearInterpolationMaterial;

template<>
InputParameters validParams<LinearInterpolationMaterial>();

class LinearInterpolationMaterial : public Material
{
public:
  LinearInterpolationMaterial(const std::string & name,
                              InputParameters parameters);

  virtual ~LinearInterpolationMaterial();

protected:
  virtual void computeQpProperties();

  bool _use_poly_fit;
  LinearInterpolation *_linear_interp;
  PolynomialFit *_poly_fit;
  MaterialProperty<Real> & _property;
};

#endif //LINEARINTERPOLATIONMATERIAL_H
