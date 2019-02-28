#include "LaplacianPressureSmootherVolumeFractionCoefMaterial.h"

registerMooseObject("THMApp", LaplacianPressureSmootherVolumeFractionCoefMaterial);

template <>
InputParameters
validParams<LaplacianPressureSmootherVolumeFractionCoefMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<Real>("Ce",
                                "User-specified tuning parameter, typically in the range 0..2");
  params.addRequiredCoupledVar("p_bar", "Average pressure value");
  params.addRequiredCoupledVar("laplace_p", "Laplacian of pressure");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity in x-direction");
  params.addRequiredParam<MaterialPropertyName>("c", "The speed of sound material property");
  params.addRequiredParam<MaterialPropertyName>("vel_int",
                                                "Interfacial velocity material property");
  params.addParam<Real>("p_reference", "Reference pressure for normalization");
  params.addParam<bool>("use_low_mach_fix", true, "Use the low-Mach fix");
  params.addRequiredParam<MaterialPropertyName>(
      "coef_name", "The material property name that will hold the dissipation coefficient");

  return params;
}

LaplacianPressureSmootherVolumeFractionCoefMaterial::
    LaplacianPressureSmootherVolumeFractionCoefMaterial(const InputParameters & parameters)
  : Material(parameters),
    _Ce(getParam<Real>("Ce")),
    _p_bar(coupledValue("p_bar")),
    _laplace_p(coupledValue("laplace_p")),
    _vI(getMaterialProperty<Real>("vel_int")),
    _vel(getMaterialProperty<Real>("vel")),
    _c(getMaterialProperty<Real>("c")),
    _use_reference_pressure(isParamValid("p_reference")),
    _p_ref(_use_reference_pressure ? getParam<Real>("p_reference") : 0.0),
    _use_low_mach_fix(getParam<bool>("use_low_mach_fix")),
    _coef(declareProperty<Real>(getParam<MaterialPropertyName>("coef_name")))
{
}

void
LaplacianPressureSmootherVolumeFractionCoefMaterial::computeQpProperties()
{
  Real h = _current_elem->hmax();
  Real p_bar = _p_bar[_qp];
  Real mag_laplace_p = std::fabs(_laplace_p[_qp]);

  Real p_normalization;
  if (_use_reference_pressure)
    p_normalization = _p_ref;
  else
    p_normalization = p_bar;

  _coef[_qp] = _Ce * h * h * h * std::fabs(_vI[_qp]) / p_normalization * mag_laplace_p;

  if (_use_low_mach_fix)
  {
    const Real M = std::fabs(_vel[_qp]) / _c[_qp];
    if (M < 1.0)
      _coef[_qp] *= M;
  }
}
