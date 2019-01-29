#include "CoupledVariableValueMaterial.h"

registerMooseObject("THMApp", CoupledVariableValueMaterial);

template <>
InputParameters
validParams<CoupledVariableValueMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "The name of the material property where we store the variable values.");
  params.addRequiredCoupledVar(
      "coupled_variable", "The coupled variable that will be stored into the material property");
  params.addClassDescription("Stores values of a variable into material properties");
  return params;
}

CoupledVariableValueMaterial::CoupledVariableValueMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _prop(declareProperty<Real>(_prop_name)),
    _value(coupledValue("coupled_variable"))
{
}

void
CoupledVariableValueMaterial::computeQpProperties()
{
  _prop[_qp] = _value[_qp];
}
