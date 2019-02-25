#ifndef INTERFACIALFRICTIONBASEMATERIAL_H
#define INTERFACIALFRICTIONBASEMATERIAL_H

#include "Material.h"
#include "FlowChannel.h"
#include "SinglePhaseFluidProperties.h"

class InterfacialFrictionBaseMaterial;

template <>
InputParameters validParams<InterfacialFrictionBaseMaterial>();

/**
 * Base class for interfacial friction coefficient implementations
 */
class InterfacialFrictionBaseMaterial : public Material
{
public:
  InterfacialFrictionBaseMaterial(const InputParameters & parameters);

protected:
  // Pipe orientation
  const bool _is_horizontal;
  // Flow channel geometry type
  FlowChannel::EConvHeatTransGeom _ht_geom;
  // pitch to diameter ratio for rod bundles
  const Real & _PoD;

  // Materials and coupled variables
  const MaterialProperty<Real> & _surface_tension;
  const MaterialProperty<Real> & _mu_l;
  const MaterialProperty<Real> & _mu_v;
  const MaterialProperty<Real> & _TI;
  const VariableValue & _alpha_vapor;
  const VariableValue & _rho_l;
  const VariableValue & _rho_v;
  const VariableValue & _T_l;
  const VariableValue & _T_v;
  const VariableValue & _v_l;
  const VariableValue & _v_v;
  const VariableValue & _p_l;
  const VariableValue & _p_v;
  const VariableValue & _D_h;

  /// Gravitational acceleration magnitude
  const Real & _gravity_magnitude;

  // Fluid properties
  const SinglePhaseFluidProperties & _fp_liquid;
  const SinglePhaseFluidProperties & _fp_vapor;

  MaterialProperty<Real> & _f_i;
};

#endif /* INTERFACIALFRICTIONBASEMATERIAL_H */
