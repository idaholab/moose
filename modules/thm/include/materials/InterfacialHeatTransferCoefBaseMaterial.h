#ifndef INTERFACIALHEATTRANSFERCOEFBASEMATERIAL_H
#define INTERFACIALHEATTRANSFERCOEFBASEMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"
#include "FlowChannel.h"

class InterfacialHeatTransferCoefBaseMaterial;
class HeatExchangeCoefficientPartitioning;
class TwoPhaseFluidProperties;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<InterfacialHeatTransferCoefBaseMaterial>();

/**
 * Base class for materials defining interfacial heat exchange coefficients
 */
class InterfacialHeatTransferCoefBaseMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  InterfacialHeatTransferCoefBaseMaterial(const InputParameters & parameters);

protected:
  virtual Real getPartition();
  virtual Real getPartitionDer();
  virtual void computeQpPropertiesZero();

  /// Cross-sectional area
  const VariableValue & _area;
  /// Hydraulic diameter
  const VariableValue & _D_h;
  /// Pipe orientation
  const bool _is_horizontal;
  /// Heat transfer geometry
  FlowChannel::EConvHeatTransGeom _ht_geom;
  /// Pitch-over-diameter ratio
  const Real & _PoD;

  const VariableValue & _beta;
  const VariableValue & _beta_dot;
  const VariableValue & _arhoA_liquid;
  const VariableValue & _arhouA_liquid;
  const VariableValue & _arhoEA_liquid;
  const VariableValue & _arhoA_vapor;
  const VariableValue & _arhouA_vapor;
  const VariableValue & _arhoEA_vapor;

  /// Liquid volume fraction
  const MaterialProperty<Real> & _alpha_liquid;
  const MaterialProperty<Real> & _dalpha_liquid_dbeta;
  /// Vapor volume fraction
  const MaterialProperty<Real> & _alpha_vapor;
  /// Liquid density
  const VariableValue & _rho_liquid;
  /// Vapor density
  const VariableValue & _rho_vapor;
  /// Liquid velocity
  const VariableValue & _vel_liquid;
  /// Vapor velocity
  const VariableValue & _vel_vapor;
  /// Liquid temperature
  const VariableValue & _T_liquid;
  /// Vapor temperature
  const VariableValue & _T_vapor;
  /// Liquid pressure
  const VariableValue & _p_liquid;
  /// Vapor pressure
  const VariableValue & _p_vapor;

  /// Surface tension
  const MaterialProperty<Real> & _surface_tension;
  /// Liquid thermal conductivity
  const MaterialProperty<Real> & _k_liquid;
  /// Vapor thermal conductivity
  const MaterialProperty<Real> & _k_vapor;
  /// Liquid dynamic viscosity
  const MaterialProperty<Real> & _mu_liquid;
  /// vapor dynamic viscosity
  const MaterialProperty<Real> & _mu_vapor;
  /// Liquid specific heat
  const MaterialProperty<Real> & _cp_liquid;
  /// Vapor specific heat
  const MaterialProperty<Real> & _cp_vapor;
  /// Liquid specific enthalpy
  const MaterialProperty<Real> & _h_liquid;
  /// Vapor specific enthalpy
  const MaterialProperty<Real> & _h_vapor;

  /// Specific interfacial area
  const MaterialProperty<Real> & _A_int;
  const MaterialProperty<Real> & _dA_int_dbeta;
  /// Interfacial temperature
  const MaterialProperty<Real> & _T_int;

  /// Heat exchange coefficeint partitioning object
  const HeatExchangeCoefficientPartitioning & _hx_part;

  /// Gravitational acceleration magnitude
  const Real & _gravity_magnitude;

  /// Two-phase fluid proeprties
  const TwoPhaseFluidProperties & _fp;
  /// Liquid fluid properties
  const SinglePhaseFluidProperties & _fp_liquid;
  /// Vapor fluid properties
  const SinglePhaseFluidProperties & _fp_vapor;

  /// Liquid volumetric heat transfer coefficeint and its derivatives wrt solution variables
  MaterialProperty<Real> & _vhtc_liquid;
  MaterialProperty<Real> & _dvhtc_liquid_dbeta;
  MaterialProperty<Real> & _dvhtc_liquid_darhoAL;
  MaterialProperty<Real> & _dvhtc_liquid_darhouAL;
  MaterialProperty<Real> & _dvhtc_liquid_darhoEAL;
  MaterialProperty<Real> & _dvhtc_liquid_darhoAV;
  MaterialProperty<Real> & _dvhtc_liquid_darhouAV;
  MaterialProperty<Real> & _dvhtc_liquid_darhoEAV;

  /// Vapor volumetric heat transfer coefficeint and its derivatives wrt solution variables
  MaterialProperty<Real> & _vhtc_vapor;
  MaterialProperty<Real> & _dvhtc_vapor_dbeta;
  MaterialProperty<Real> & _dvhtc_vapor_darhoAL;
  MaterialProperty<Real> & _dvhtc_vapor_darhouAL;
  MaterialProperty<Real> & _dvhtc_vapor_darhoEAL;
  MaterialProperty<Real> & _dvhtc_vapor_darhoAV;
  MaterialProperty<Real> & _dvhtc_vapor_darhouAV;
  MaterialProperty<Real> & _dvhtc_vapor_darhoEAV;
};

#endif /* INTERFACIALHEATTRANSFERCOEFBASEMATERIAL_H */
