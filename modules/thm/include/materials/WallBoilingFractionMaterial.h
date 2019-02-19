#ifndef WALLBOILINGFRACTIONMATERIAL_H
#define WALLBOILINGFRACTIONMATERIAL_H

#include "DerivativeMaterialInterfaceTHM.h"

class WallBoilingFractionMaterial;

template <>
InputParameters validParams<WallBoilingFractionMaterial>();

/**
 * Computes the fraction \f$f_{boil}\f$ of liquid wall heat flux that goes to
 * boiling
 *
 * This coefficient is such that
 * \f[
 *   q_{boil} = f_{boil} q_{wall,\ell}
 * \f]
 * \f[
 *   q_{conv} = (1 - f_{boil}) q_{wall,\ell}
 * \f]
 */
class WallBoilingFractionMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  WallBoilingFractionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Wall boiling fraction
  MaterialProperty<Real> & _f_boil;
  MaterialProperty<Real> & _df_boil_dbeta;
  MaterialProperty<Real> & _df_boil_darhoA;
  MaterialProperty<Real> & _df_boil_darhouA;
  MaterialProperty<Real> & _df_boil_darhoEA;

  /// Wall temperature
  const MaterialProperty<Real> & _T_wall;

  /// Liquid temperature
  const MaterialProperty<Real> & _T_liquid;

  /// Saturation temperature at liquid pressure
  const MaterialProperty<Real> & _T_sat_liquid;
  const MaterialProperty<Real> & _dT_sat_liquid_dbeta;
  const MaterialProperty<Real> & _dT_sat_liquid_darhoA;
  const MaterialProperty<Real> & _dT_sat_liquid_darhouA;
  const MaterialProperty<Real> & _dT_sat_liquid_darhoEA;
};

#endif /* WALLBOILINGFRACTIONMATERIAL_H */
