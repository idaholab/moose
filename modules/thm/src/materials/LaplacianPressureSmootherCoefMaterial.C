#include "LaplacianPressureSmootherCoefMaterial.h"

registerMooseObject("THMApp", LaplacianPressureSmootherCoefMaterial);

template <>
InputParameters
validParams<LaplacianPressureSmootherCoefMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<Real>("Ce", "User specified constant: typically 0..2");
  params.addRequiredParam<MaterialPropertyName>("c", "The speed of sound material property");
  params.addRequiredCoupledVar("vel", "Velocity in x-direction");
  params.addRequiredCoupledVar("p_bar", "Average pressure value");
  params.addRequiredCoupledVar("laplace_p", "Laplacian of pressure");
  params.addParam<Real>("p_reference", "Reference pressure for normalization");
  params.addParam<bool>("use_low_mach_fix", true, "Use the low-Mach fix");
  params.addRequiredParam<MaterialPropertyName>(
      "coef_name", "The material property name that will hold the dissipation coefficient");
  return params;
}

LaplacianPressureSmootherCoefMaterial::LaplacianPressureSmootherCoefMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Ce(getParam<Real>("Ce")),
    _vel(coupledValue("vel")),
    _c(getMaterialProperty<Real>("c")),
    _p_bar(coupledValue("p_bar")),
    _laplace_p(coupledValue("laplace_p")),
    _use_reference_pressure(isParamValid("p_reference")),
    _p_ref(_use_reference_pressure ? getParam<Real>("p_reference") : 0.0),
    _use_low_mach_fix(getParam<bool>("use_low_mach_fix")),
    _coef(declareProperty<Real>(getParam<MaterialPropertyName>("coef_name")))
{
}

void
LaplacianPressureSmootherCoefMaterial::computeQpProperties()
{
  Real h = _current_elem->hmax();
  Real mag_vel = std::fabs(_vel[_qp]);
  Real c = _c[_qp];

  Real p_normalization;
  if (_use_reference_pressure)
    p_normalization = _p_ref;
  else
    p_normalization = _p_bar[_qp];

  Real mag_laplace_p =
      std::fabs(_laplace_p[_qp]); // 1D version, multi dimensional is sqrt(\grad p * \grad p]);

  _coef[_qp] = _Ce * h * h * h * ((mag_vel + c) / p_normalization) * mag_laplace_p;

  if (_use_low_mach_fix)
  {
    const Real M = mag_vel / c;
    if (M < 1.0)
      _coef[_qp] *= M;
  }
}
