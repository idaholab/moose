#ifndef DEFAULTMATPROPCONSUMERMATERIAL_H
#define DEFAULTMATPROPCONSUMERMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward declarations
class DefaultMatPropConsumerMaterial;

template <>
InputParameters validParams<DefaultMatPropConsumerMaterial>();

class DefaultMatPropConsumerMaterial : public DerivativeMaterialInterface<Material>
{
public:
  DefaultMatPropConsumerMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  std::string _prop_name;
  const MaterialProperty<Real> & _prop;
};

#endif // DEFAULTMATPROPCONSUMERMATERIAL_H
