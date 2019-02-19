#ifndef WALLHEATTRANSFERCOEFFICIENT7EQNBASEMATERIAL_H
#define WALLHEATTRANSFERCOEFFICIENT7EQNBASEMATERIAL_H

#include "Material.h"
#include "PipeBase.h"

class WallHeatTransferCoefficient7EqnBaseMaterial;
class TwoPhaseFluidProperties;
class SinglePhaseFluidProperties;
class LiquidFluidPropertiesInterface;
class CHFTable;

template <>
InputParameters validParams<WallHeatTransferCoefficient7EqnBaseMaterial>();

/**
 * Base class for computing wall heat transfer coefficient for pipe and rod bundle geometry for
 * two phase flow
 */
class WallHeatTransferCoefficient7EqnBaseMaterial : public Material
{
public:
  WallHeatTransferCoefficient7EqnBaseMaterial(const InputParameters & parameters);

protected:
  MaterialProperty<Real> & _Hw_liquid;
  MaterialProperty<Real> & _Hw_vapor;

  PipeBase::EConvHeatTransGeom _ht_geom;
  const Real & _PoD;
  /// vapor volume fraction that is all liquid
  const Real & _alpha_v_min;
  /// vapor volume fraction that is all vapor
  const Real & _alpha_v_max;

  const VariableValue & _arhoA_liquid;
  const VariableValue & _arhoA_vapor;
  const VariableValue & _area;
  const VariableValue & _D_h;
  const MaterialProperty<Real> & _alpha_liquid;
  const MaterialProperty<Real> & _alpha_vapor;
  const MaterialProperty<Real> & _rho_liquid;
  const MaterialProperty<Real> & _rho_vapor;
  const MaterialProperty<Real> & _p_liquid;
  const MaterialProperty<Real> & _p_vapor;
  const MaterialProperty<Real> & _vel_liquid;
  const MaterialProperty<Real> & _vel_vapor;
  const MaterialProperty<Real> & _v_liquid;
  const MaterialProperty<Real> & _v_vapor;
  const MaterialProperty<Real> & _e_liquid;
  const MaterialProperty<Real> & _e_vapor;
  const MaterialProperty<Real> & _T_liquid;
  const MaterialProperty<Real> & _T_vapor;
  const MaterialProperty<Real> & _T_wall;
  const MaterialProperty<Real> & _T_sat_liquid;
  bool _has_q_wall;
  const VariableValue * _q_wall;

  /// Gravitational acceleration magnitude
  const Real & _gravity_magnitude;

  const TwoPhaseFluidProperties & _fp;
  const SinglePhaseFluidProperties & _fp_liquid;
  const SinglePhaseFluidProperties & _fp_vapor;
  const LiquidFluidPropertiesInterface & _liquid_props;

  const CHFTable & _chf_table;
};

#endif /* WALLHEATTRANSFERCOEFFICIENT7EQNBASEMATERIAL_H */
