#include "ADMaterialFunctionProductMaterial.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADMaterialFunctionProductMaterial);

InputParameters
ADMaterialFunctionProductMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("mat_prop_product", "Product material property");
  params.addRequiredParam<MaterialPropertyName>("mat_prop_scale", "Scale material property");
  params.addRequiredParam<FunctionName>("function", "Function in product");

  params.addClassDescription("Computes the product of a material property and a function.");

  return params;
}

ADMaterialFunctionProductMaterial::ADMaterialFunctionProductMaterial(
    const InputParameters & parameters)
  : Material(parameters),

    _product(declareADProperty<Real>(getParam<MaterialPropertyName>("mat_prop_product"))),
    _scale(getADMaterialProperty<Real>("mat_prop_scale")),
    _function(getFunction("function"))
{
}

void
ADMaterialFunctionProductMaterial::computeQpProperties()
{
  _product[_qp] = _scale[_qp] * _function.value(_t, _q_point[_qp]);
}
