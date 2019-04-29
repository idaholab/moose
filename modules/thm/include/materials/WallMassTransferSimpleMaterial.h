#pragma once

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"
#include "Enums.h"

// Forward Declarations
class WallMassTransferSimpleMaterial;
class TwoPhaseFluidProperties;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<WallMassTransferSimpleMaterial>();

/**
 * This material class calculates the mass transfer coefficient.
 */
class WallMassTransferSimpleMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  WallMassTransferSimpleMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Heat flux perimeter
  const VariableValue & _P_hf;

  /// Wall temperature
  const MaterialProperty<Real> & _T_wall;
  /// Heat convective transfer coefficient of liquid
  const MaterialProperty<Real> & _Hw_liquid;
  /// Heat flux partitioning coefficient of liquid
  const MaterialProperty<Real> & _kappa_liquid;
  const MaterialProperty<Real> & _dkappa_liquid_dbeta;

  /// fluid properties
  const TwoPhaseFluidProperties & _tpfp;
  const SinglePhaseFluidProperties & _fp_liquid;
  const SinglePhaseFluidProperties & _fp_vapor;

  /// Wall mass transfer coefficient and its derivatives
  const MaterialPropertyName _Gamma_wall_name;
  MaterialProperty<Real> & _Gamma_wall;
  MaterialProperty<Real> & _d_Gamma_wall_dbeta;
  MaterialProperty<Real> & _d_Gamma_wall_darhoAL;
  MaterialProperty<Real> & _d_Gamma_wall_darhouAL;
  MaterialProperty<Real> & _d_Gamma_wall_darhoEAL;
  MaterialProperty<Real> & _d_Gamma_wall_darhoAV;
  MaterialProperty<Real> & _d_Gamma_wall_darhouAV;
  MaterialProperty<Real> & _d_Gamma_wall_darhoEAV;

  /// Temperature of liquid
  const MaterialProperty<Real> & _T_liquid;
  const MaterialProperty<Real> & _dTL_dbeta;
  const MaterialProperty<Real> & _dTL_darhoAL;
  const MaterialProperty<Real> & _dTL_darhouAL;
  const MaterialProperty<Real> & _dTL_darhoEAL;

  /// Average interfacial pressure
  const MaterialProperty<Real> & _pIbar;
  const MaterialProperty<Real> & _dpIbar_dbeta;
  const MaterialProperty<Real> & _dpIbar_darhoAL;
  const MaterialProperty<Real> & _dpIbar_darhouAL;
  const MaterialProperty<Real> & _dpIbar_darhoEAL;
  const MaterialProperty<Real> & _dpIbar_darhoAV;
  const MaterialProperty<Real> & _dpIbar_darhouAV;
  const MaterialProperty<Real> & _dpIbar_darhoEAV;

  /// Pressure of liquid
  const MaterialProperty<Real> & _p_liquid;
  const MaterialProperty<Real> & _dpL_dbeta;
  const MaterialProperty<Real> & _dpL_darhoAL;
  const MaterialProperty<Real> & _dpL_darhouAL;
  const MaterialProperty<Real> & _dpL_darhoEAL;

  /// Wall boiling fraction
  const MaterialProperty<Real> & _f_boil;
  const MaterialProperty<Real> & _df_boil_dbeta;
  const MaterialProperty<Real> & _df_boil_darhoA_liquid;
  const MaterialProperty<Real> & _df_boil_darhouA_liquid;
  const MaterialProperty<Real> & _df_boil_darhoEA_liquid;
};
