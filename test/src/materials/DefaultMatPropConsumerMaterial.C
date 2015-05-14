#include "DefaultMatPropConsumerMaterial.h"

template<>
InputParameters validParams<DefaultMatPropConsumerMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("mat_prop", "prop", "Material property name to fetch");
  return params;
}

DefaultMatPropConsumerMaterial::DefaultMatPropConsumerMaterial(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
		_prop_name(getParam<std::string>("mat_prop")),
		_prop(getDefaultMaterialProperty<Real>(_prop_name))
{
}
