#include "GradientPressureSmootherVolumeFractionCoefMaterial.h"

registerMooseObject("THMApp", GradientPressureSmootherVolumeFractionCoefMaterial);

template <>
InputParameters
validParams<GradientPressureSmootherVolumeFractionCoefMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<Real>("Ce",
                                "User-specified tuning parameter, typically in the range (0,2)");
  params.addRequiredCoupledVar("p_bar", "Average pressure value");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("vel", "Velocity in x-direction");
  params.addRequiredParam<MaterialPropertyName>("c", "The speed of sound material property");
  params.addRequiredParam<MaterialPropertyName>("vel_int",
                                                "Interfacial velocity material property");
  params.addParam<Real>("p_reference", "Reference pressure for normalization");
  params.addParam<bool>("use_low_mach_fix", true, "Use the low-Mach fix");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>(
      "coef_name", "The material property name that will hold the dissipation coefficient");

  return params;
}

GradientPressureSmootherVolumeFractionCoefMaterial::
    GradientPressureSmootherVolumeFractionCoefMaterial(const InputParameters & parameters)
  : Material(parameters),
    _Ce(getParam<Real>("Ce")),
    _p_bar(coupledValue("p_bar")),
    _grad_p(coupledGradient("p")),
    _vI(getMaterialProperty<Real>("vel_int")),
    _vel(coupledValue("vel")),
    _c(getMaterialProperty<Real>("c")),
    _use_reference_pressure(isParamValid("p_reference")),
    _p_ref(_use_reference_pressure ? getParam<Real>("p_reference") : 0.0),
    _use_low_mach_fix(getParam<bool>("use_low_mach_fix")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _coef(declareProperty<Real>(getParam<MaterialPropertyName>("coef_name")))
{
}

void
GradientPressureSmootherVolumeFractionCoefMaterial::computeQpProperties()
{
  Real h = _current_elem->hmax();
  Real mag_grad_p = std::fabs(
      _grad_p[_qp] * _dir[_qp]); // 1D version, multi dimensional is sqrt(\grad p * \grad p]);

  Real p_normalization;
  if (_use_reference_pressure)
    p_normalization = _p_ref;
  else
    p_normalization = _p_bar[_qp];

  _coef[_qp] = _Ce * h * h * (std::fabs(_vI[_qp]) / p_normalization) * mag_grad_p;

  if (_use_low_mach_fix)
  {
    const Real M = std::fabs(_vel[_qp]) / _c[_qp];
    if (M < 1.0)
      _coef[_qp] *= M;
  }
}
