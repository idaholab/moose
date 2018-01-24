#include "DefaultMatPropConsumerKernel.h"

template <>
InputParameters
validParams<DefaultMatPropConsumerKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<MaterialPropertyName>("mat_prop", "prop", "Material property name to fetch");
  return params;
}

DefaultMatPropConsumerKernel::DefaultMatPropConsumerKernel(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _prop(getDefaultMaterialProperty<Real>("mat_prop"))
{
}
