#include "ElectricalConductivity.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ElectricalConductivity>()
{
  InputParameters params = validParams<Material>();
  params.addCoupledVar("temp", 300.0, "variable for temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<Real>("length_scale", 1.0e-9, "Lengthscale of model");
  params.addParam<Real>(
      "ref_resistivity",
      6.5e-6,
      "Electrical resistivity of the material at reference temperature in ohm-m.");
  params.addParam<Real>(
      "temp_coeff",
      0.0045,
      "Temperature coefficient for calculating dependence of resistivity on temp.");
  params.addParam<Real>("ref_temp", 300.0, "Reference temperature for Electrical resistivity in K");
  return params;
}

ElectricalConductivity::ElectricalConductivity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _length_scale(getParam<Real>("length_scale")),
    _ref_resis(getParam<Real>("ref_resistivity")),
    _temp_coeff(getParam<Real>("temp_coeff")),
    _ref_temp(getParam<Real>("ref_temp")),
    _T(coupledValue("temp")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _electric_conductivity(declareProperty<Real>(_base_name + "electrical_conductivity")),
    _delectric_conductivity_dT(declarePropertyDerivative<Real>(
        _base_name + "electrical_conductivity", getVar("temp", 0)->name()))
{
}

void
ElectricalConductivity::computeQpProperties()
{
  const Real ref_resis = _ref_resis / _length_scale;
  const Real resistivity = ref_resis * (1.0 + _temp_coeff * (_T[_qp] - _ref_temp));
  const Real dresistivity_dT = ref_resis * _temp_coeff;
  _electric_conductivity[_qp] = 1.0 / resistivity;
  _delectric_conductivity_dT[_qp] = -1.0 / (resistivity * resistivity) * dresistivity_dT;
}
