
#pragma once

#include "Material.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes fluid properties using (pressure, enthalpy) formulation
 * for HEM material !!!
 */
class ADFluidPropertiesMaterialPh : public Material
{
public:
  static InputParameters validParams();

  ADFluidPropertiesMaterialPh(const InputParameters & parameters);
  virtual ~ADFluidPropertiesMaterialPh();

protected:
  virtual void computeQpProperties();

  /// 3 传入参数: 压力，焓值，速度//////////////////////////////////////////////////////////////////////

  /// Pressure (Pa)
  const ADVariableValue & _pressure;
  /// gradient of the pressure
  const ADVariableGradient & _grad_p;
  /// Specific enthalpy (J/kg)
  // 注意单位不是kJ   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  const ADVariableValue & _enthalpy;
  const ADVariableGradient & _grad_enthalpy;
  /// velocity
  const ADVectorVariableValue & _velocity;
  /// gradient of velocity
  const ADVectorVariableGradient & _grad_velocity;

  /// 待求量//////////////////////////////////////////////////////////////////////

  /// Density (kg/m^3)
  ADMaterialProperty<Real> & _rho;
  // 温度 K
  ADMaterialProperty<Real> & _T;
  // 密度对压力和焓值的偏导
  ADMaterialProperty<Real> & _drho_dp;
  ADMaterialProperty<Real> & _drho_dh;


  ADMaterialProperty<Real> & _mu;

  /// 动量对流项接口
  // ADMaterialProperty<RealVectorValue> & _advective_strong_residual;
  // 能量对流项接口
  // ADMaterialProperty<RealVectorValue> & _enthalpy_advective_strong_residual;

  // // 传入瞬态工况的判断/////////////////////////////////////////////
  // bool _has_transient;
  //
  // /// 速度和焓值对时间的偏导：du/dt，dh/dt
  // const ADVectorVariableValue * _velocity_dot;
  // const ADVariableValue * _enthalpy_dot;
  //
  // // 动量瞬态项接口
  // ADMaterialProperty<RealVectorValue> & _td_strong_residual;
  // // 能量瞬态项接口
  // ADMaterialProperty<Real> & _enthalpy_td_strong_residual;

  /// Fluid properties UserObject
  const SinglePhaseFluidProperties & _fp;
};
