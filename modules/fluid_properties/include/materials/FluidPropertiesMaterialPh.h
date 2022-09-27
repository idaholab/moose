
#pragma once

#include "Material.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes fluid properties using (pressure, enthalpy) formulation
 * for HEM material !!!
 */
class FluidPropertiesMaterialPh : public Material
{
public:
  static InputParameters validParams();

  FluidPropertiesMaterialPh(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterialPh();

protected:
  virtual void computeQpProperties();

  /// 传入参数
  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Specific enthalpy (J/kg)
  // 注意单位不是kJ   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  const VariableValue & _enthalpy;

  /// 待求量
  /// Density (kg/m^3)
  MaterialProperty<Real> & _rho;
  MaterialProperty<Real> & _T;

  MaterialProperty<Real> & _drho_dp;
  MaterialProperty<Real> & _drho_dh;

  /// Fluid properties UserObject
  const SinglePhaseFluidProperties & _fp;
};
