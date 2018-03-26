#include "ConstantMaterial.h"

registerMooseObject("RELAP7App", ConstantMaterial);

template <>
InputParameters
validParams<ConstantMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("value", 0., "Constant value being assigned into the property");
  params.addRequiredParam<std::string>("property_name", "The property name to declare");
  params.addCoupledVar(
      "derivative_vars",
      "Names of variables for which to create (zero) material derivative properties");
  return params;
}

ConstantMaterial::ConstantMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _value(getParam<Real>("value")),
    _property_name(getParam<std::string>("property_name")),
    _property(declareProperty<Real>(_property_name)),
    _n_derivative_vars(coupledComponents("derivative_vars"))
{
  // get references to new material property derivatives
  _derivative_properties.resize(_n_derivative_vars);
  for (unsigned int i = 0; i < _n_derivative_vars; ++i)
    _derivative_properties[i] =
        &declarePropertyDerivative<Real>(_property_name, getVar("derivative_vars", i)->name());
}

void
ConstantMaterial::computeQpProperties()
{
  _property[_qp] = _value;

  for (unsigned int i = 0; i < _n_derivative_vars; ++i)
    (*_derivative_properties[i])[_qp] = 0;
}
