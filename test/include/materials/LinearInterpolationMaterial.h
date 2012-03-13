#ifndef LINEARINTERPOLATIONMATERIAL_H
#define LINEARINTERPOLATIONMATERIAL_H

#include "Material.h"

#include "LinearInterpolation.h"

class LinearInterpolationMaterial;

template<>
InputParameters validParams<LinearInterpolationMaterial>();

class LinearInterpolationMaterial : public Material
{
public:
  LinearInterpolationMaterial(const std::string & name,
                InputParameters parameters);

protected:
  virtual void computeQpProperties();

  LinearInterpolation _piecewise_func;
  MaterialProperty<Real> & _property;
};

#endif //LINEARINTERPOLATIONMATERIAL_H
