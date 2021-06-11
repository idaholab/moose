#include "ADWallFrictionFunctionMaterial.h"
#include "Function.h"

registerMooseObject("THMApp", ADWallFrictionFunctionMaterial);

InputParameters
ADWallFrictionFunctionMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");

  params.addRequiredParam<FunctionName>("function", "Darcy friction factor function");

  return params;
}

ADWallFrictionFunctionMaterial::ADWallFrictionFunctionMaterial(const InputParameters & parameters)
  : Material(parameters),

    _function(getFunction("function")),

    _f_D_name(getParam<MaterialPropertyName>("f_D")),
    _f_D(declareADProperty<Real>(_f_D_name))
{
}

void
ADWallFrictionFunctionMaterial::computeQpProperties()
{
  _f_D[_qp] = _function.value(_t, _q_point[_qp]);
}
