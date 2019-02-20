#include "WallBoilingFractionMaterial.h"

registerMooseObject("THMApp", WallBoilingFractionMaterial);

template <>
InputParameters
validParams<WallBoilingFractionMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredCoupledVar("beta", "Remapped volume fraction of liquid");
  params.addRequiredCoupledVar("arhoA_liquid", "Liquid mass equation variable: alpha*rho*A");
  params.addRequiredCoupledVar("arhouA_liquid", "Liquid momentum equation variable: alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA_liquid", "Liquid energy equation variable: alpha*rho*E*A");

  params.addRequiredParam<MaterialPropertyName>("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>("T_liquid", "Liquid temperature");
  params.addRequiredParam<MaterialPropertyName>("T_sat_liquid",
                                                "Saturation temperature at liquid pressure");

  return params;
}

WallBoilingFractionMaterial::WallBoilingFractionMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),

    _f_boil(declareProperty<Real>("wall_boiling_fraction")),
    _df_boil_dbeta(declarePropertyDerivativeTHM<Real>("wall_boiling_fraction", "beta")),
    _df_boil_darhoA(declarePropertyDerivativeTHM<Real>("wall_boiling_fraction", "arhoA_liquid")),
    _df_boil_darhouA(declarePropertyDerivativeTHM<Real>("wall_boiling_fraction", "arhouA_liquid")),
    _df_boil_darhoEA(declarePropertyDerivativeTHM<Real>("wall_boiling_fraction", "arhoEA_liquid")),

    _T_wall(getMaterialProperty<Real>("T_wall")),
    _T_liquid(getMaterialProperty<Real>("T_liquid")),

    _T_sat_liquid(getMaterialProperty<Real>("T_sat_liquid")),
    _dT_sat_liquid_dbeta(getMaterialPropertyDerivativeTHM<Real>("T_sat_liquid", "beta")),
    _dT_sat_liquid_darhoA(getMaterialPropertyDerivativeTHM<Real>("T_sat_liquid", "arhoA_liquid")),
    _dT_sat_liquid_darhouA(getMaterialPropertyDerivativeTHM<Real>("T_sat_liquid", "arhouA_liquid")),
    _dT_sat_liquid_darhoEA(getMaterialPropertyDerivativeTHM<Real>("T_sat_liquid", "arhoEA_liquid"))
{
}

void
WallBoilingFractionMaterial::computeQpProperties()
{
  if (_T_wall[_qp] > _T_sat_liquid[_qp] && _T_wall[_qp] > _T_liquid[_qp])
  {
    _f_boil[_qp] = 1 - std::exp(-0.25 * (_T_wall[_qp] - _T_sat_liquid[_qp]));

    const Real df_boil_dT_sat_liquid =
        -0.25 * std::exp(-0.25 * (_T_wall[_qp] - _T_sat_liquid[_qp]));

    _df_boil_dbeta[_qp] = df_boil_dT_sat_liquid * _dT_sat_liquid_dbeta[_qp];
    _df_boil_darhoA[_qp] = df_boil_dT_sat_liquid * _dT_sat_liquid_darhoA[_qp];
    _df_boil_darhouA[_qp] = df_boil_dT_sat_liquid * _dT_sat_liquid_darhouA[_qp];
    _df_boil_darhoEA[_qp] = df_boil_dT_sat_liquid * _dT_sat_liquid_darhoEA[_qp];
  }
  else
  {
    _f_boil[_qp] = 0;
    _df_boil_dbeta[_qp] = 0;
    _df_boil_darhoA[_qp] = 0;
    _df_boil_darhouA[_qp] = 0;
    _df_boil_darhoEA[_qp] = 0;
  }
}
