#include "WallFrictionFunctionMaterial.h"
#include "Function.h"

registerMooseObject("THMApp", WallFrictionFunctionMaterial);

template <>
InputParameters
validParams<WallFrictionFunctionMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");

  params.addRequiredParam<FunctionName>("function", "Darcy friction factor function");

  params.addCoupledVar("beta", "Volume fraction equation variable: beta");
  params.addRequiredCoupledVar("arhoA", "Mass equation variable: alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "Momentum equation variable: alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "Energy equation variable: alpha*rho*E*A");

  return params;
}

WallFrictionFunctionMaterial::WallFrictionFunctionMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),

    _function(getFunction("function")),

    _f_D_name(getParam<MaterialPropertyName>("f_D")),
    _f_D(declareProperty<Real>(_f_D_name)),
    _df_D_dbeta(isCoupled("beta") ? &declarePropertyDerivativeTHM<Real>(_f_D_name, "beta")
                                  : nullptr),
    _df_D_darhoA(declarePropertyDerivativeTHM<Real>(_f_D_name, "arhoA")),
    _df_D_darhouA(declarePropertyDerivativeTHM<Real>(_f_D_name, "arhouA")),
    _df_D_darhoEA(declarePropertyDerivativeTHM<Real>(_f_D_name, "arhoEA"))
{
}

void
WallFrictionFunctionMaterial::computeQpProperties()
{
  _f_D[_qp] = _function.value(_t, _q_point[_qp]);

  _df_D_darhoA[_qp] = 0;
  _df_D_darhouA[_qp] = 0;
  _df_D_darhoEA[_qp] = 0;
  if (isCoupled("beta"))
    (*_df_D_dbeta)[_qp] = 0;
}
