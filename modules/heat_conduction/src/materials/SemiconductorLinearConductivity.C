#include "SemiconductorLinearConductivity.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<SemiconductorLinearConductivity>()
{
  InputParameters params = validParams<Material>();
  params.addCoupledVar("temp", 300.0, "variable for temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addRequiredParam<Real>(
      "sh_coeff_A",
      "Steinhart_Hart coefficient A of the material, for electrical conductivity in 1/(ohm-m).");
  params.addRequiredParam<Real>(
      "sh_coeff_B",
      "Steinhart_Hart coefficient B of the material, for electrical conductivity in 1/(ohm-m).");
  return params;
}

SemiconductorLinearConductivity::SemiconductorLinearConductivity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _sh_coeff_A(getParam<Real>("sh_coeff_A")),
    _sh_coeff_B(getParam<Real>("sh_coeff_B")),
    _T(coupledValue("temp")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _electric_conductivity(declareProperty<Real>(_base_name + "electrical_conductivity")),
    _delectric_conductivity_dT(declarePropertyDerivative<Real>(
        _base_name + "electrical_conductivity", getVar("temp", 0)->name()))
{
}

void
SemiconductorLinearConductivity::computeQpProperties()
{
  _electric_conductivity[_qp] = exp((_sh_coeff_A - 1/ _T[_qp])/_sh_coeff_B);
  _delectric_conductivity_dT[_qp] = _electric_conductivity[_qp]/(_sh_coeff_B * _T[_qp]*_T[_qp]);
}
