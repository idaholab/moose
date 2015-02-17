#include "DerivativeBaseMaterial.h"

template<>
InputParameters validParams<DerivativeBaseMaterial>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  return params;
}

DerivativeBaseMaterial::DerivativeBaseMaterial(const std::string & name,
                                               InputParameters parameters) :
    FunctionMaterialBase(name, parameters),
    DerivativeFunctionMaterialBase(name, parameters)
{
}
