#ifndef FLOWREGIMEBASEMATERIAL_H
#define FLOWREGIMEBASEMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"
#include "PipeBase.h"

// Forward Declarations
class FlowRegimeBaseMaterial;
class TwoPhaseFluidProperties;
class SinglePhaseFluidProperties;
class CHFTable;

template <>
InputParameters validParams<FlowRegimeBaseMaterial>();

/**
 * Base class for computing flow regime maps
 */
class FlowRegimeBaseMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  FlowRegimeBaseMaterial(const InputParameters & parameters);

protected:
  /// Cross-sectional area
  const VariableValue & _area;
  /// Hydraulic diameter
  const VariableValue & _D_h;
  /// Heat transfer geometry
  PipeBase::EConvHeatTransGeom _ht_geom;
  /// pitch to diameter ratio for rod bundles
  const Real & _PoD;

  /// Liquid volume fraction
  const VariableValue & _alpha_liquid;
  const MaterialProperty<Real> & _dalpha_liquid_dbeta;
  /// Vapor volume fraction
  const VariableValue & _alpha_vapor;
  const VariableValue & _arhoA_liquid;
  const VariableValue & _arhoA_vapor;
  /// Liquid specific internal energy
  const VariableValue & _e_liquid;
  /// Vapor specific internal energy
  const VariableValue & _e_vapor;
  /// Liquid phase velocity
  const VariableValue & _velocity_liquid;
  /// Vapor phase velocity
  const VariableValue & _velocity_vapor;
  /// Liquid phase density
  const VariableValue & _rho_liquid;
  /// Vapor phase density
  const VariableValue & _rho_vapor;
  /// Spcific volume of liquid phase
  const VariableValue & _v_liquid;
  /// Specific volume of vapor phase
  const VariableValue & _v_vapor;
  /// true if heat flux was provided, otherwise false
  bool _has_q_wall;
  /// Variable providing heat flux
  const VariableValue * _q_wall;
  /// Vapor volume fraction that is all liquid
  const Real & _alpha_v_min;
  /// Vapor volume fraction that is all vapor
  const Real & _alpha_v_max;
  /// Angle between orientation vector and gravity vector, in degrees
  const Real & _gravity_angle;
  /// Flag that determines if the flow channel has horizontal or vertical direction
  const bool _is_horizontal;

  /// Liquid dynamic viscosity
  const MaterialProperty<Real> & _mu_liquid;
  /// Vapor dynamic viscosity
  const MaterialProperty<Real> & _mu_vapor;
  /// Liquid specific heat
  const MaterialProperty<Real> & _cp_liquid;
  /// Vapor specific heat
  const MaterialProperty<Real> & _cp_vapor;
  /// Liquid thermal conductivity
  const MaterialProperty<Real> & _k_liquid;
  /// Vapor thermal conductivity
  const MaterialProperty<Real> & _k_vapor;
  /// Liquid pressure
  const MaterialProperty<Real> & _p_liquid;
  /// Vapor pressure
  const MaterialProperty<Real> & _p_vapor;
  /// Liquid temperature
  const MaterialProperty<Real> & _T_liquid;
  /// Vapor temperature
  const MaterialProperty<Real> & _T_vapor;
  /// Surface tension
  const MaterialProperty<Real> & _surface_tension;
  /// Liquid specific enthalpy
  const MaterialProperty<Real> & _h_liquid;
  /// Vapor specific enthalpy
  const MaterialProperty<Real> & _h_vapor;
  /// Heat flux partitioning coefficient
  MaterialProperty<Real> & _kappa_liquid;
  MaterialProperty<Real> & _dkappa_liquid_dbeta;

  /// Gravitational acceleration magnitude
  const Real & _gravity_magnitude;

  /// User object providing 2-phase fluid properties
  const TwoPhaseFluidProperties & _fp;
  /// User object providing liquid fluid properties
  const SinglePhaseFluidProperties & _fp_liquid;
  /// User object providing vapor fluid properties
  const SinglePhaseFluidProperties & _fp_vapor;

  /// Critical heat flux table used
  const CHFTable & _chf_table;

  /// Fluid properties support mass transfer and mass transfer is enabled
  const bool _mass_transfer;
};

#endif /* FLOWREGIMEBASEMATERIAL_H */
