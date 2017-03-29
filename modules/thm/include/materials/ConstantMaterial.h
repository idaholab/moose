#ifndef CONSTANTMATERIAL_H
#define CONSTANTMATERIAL_H

#include "Material.h"

class ConstantMaterial;

template <>
InputParameters validParams<ConstantMaterial>();

/**
 *
 */
class ConstantMaterial : public Material
{
public:
  ConstantMaterial(const InputParameters & parameters);
  virtual ~ConstantMaterial();

protected:
  virtual void computeQpProperties();

  const Real & _value;
  MaterialProperty<Real> & _property;
};

#endif /* CONSTANTMATERIAL_H */
