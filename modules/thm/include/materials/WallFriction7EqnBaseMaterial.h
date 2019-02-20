#ifndef WALLFRICTION7EQNBASEMATERIAL_H
#define WALLFRICTION7EQNBASEMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"
#include "PipeBase.h"

class WallFriction7EqnBaseMaterial;

template <>
InputParameters validParams<WallFriction7EqnBaseMaterial>();

/**
 * Base class for computing wall friction coefficients for liquid and vapor phase
 */
class WallFriction7EqnBaseMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  WallFriction7EqnBaseMaterial(const InputParameters & parameters);

protected:
  // Pipe orientation
  const bool _is_horizontal;
  // Flow channel geometry type
  const PipeBase::EConvHeatTransGeom _ht_geom;

  MaterialProperty<Real> & _f_D_liquid;
  MaterialProperty<Real> & _df_D_liquid_dbeta;
  MaterialProperty<Real> & _df_D_liquid_drhoAL;
  MaterialProperty<Real> & _df_D_liquid_drhouAL;
  MaterialProperty<Real> & _df_D_liquid_drhoEAL;

  MaterialProperty<Real> & _f_D_vapor;
  MaterialProperty<Real> & _df_D_vapor_dbeta;
  MaterialProperty<Real> & _df_D_vapor_drhoAV;
  MaterialProperty<Real> & _df_D_vapor_drhouAV;
  MaterialProperty<Real> & _df_D_vapor_drhoEAV;

  const MaterialProperty<Real> & _mu_l;
  const MaterialProperty<Real> & _mu_v;
  const MaterialProperty<Real> & _surface_tension;

  /// Coupled variable values
  const VariableValue & _alpha_liquid;
  const VariableValue & _alpha_vapor;
  const VariableValue & _rho_l;
  const VariableValue & _rho_v;
  const VariableValue & _T_l;
  const VariableValue & _v_l;
  const VariableValue & _v_v;
  const VariableValue & _D_h;
  const Real & _roughness;

  const MaterialProperty<bool> & _is_post_CHF;
  const MaterialProperty<std::vector<Real>> & _wdrag_flow_regime;
  const MaterialProperty<std::vector<Real>> & _wht_flow_regime;
};

#endif /* WALLFRICTION7EQNBASEMATERIAL_H */
