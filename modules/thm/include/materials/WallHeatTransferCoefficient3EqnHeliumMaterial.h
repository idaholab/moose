#ifndef WALLHEATTRANSFERCOEFFICIENT3EQNHELIUMMATERIAL_H
#define WALLHEATTRANSFERCOEFFICIENT3EQNHELIUMMATERIAL_H

#include "WallHeatTransferCoefficient3EqnBaseMaterial.h"

class WallHeatTransferCoefficient3EqnHeliumMaterial;

template <>
InputParameters validParams<WallHeatTransferCoefficient3EqnHeliumMaterial>();

/**
 * Computes wall heat transfer coefficient for helium using McEligot et al. (1966) correlation
 *
 * See McEligot D.M. et al., “Investigation of Fundamental Thermal-Hydraulic Phenomena in Advanced
 * Gas-Cooled Reactors,” INL/EXT-06-11801, September 2006, equation (2-4)
 *
 * Valid for x/D > 30, 4000 < Re < 15000, 0 < q* < 0.004, where q* is "nondimensional heat flux",
 * defined as q" / (G h), where G is mass flux [kg/(m2 -s)] and h is specific enthalpy.
 */
class WallHeatTransferCoefficient3EqnHeliumMaterial
  : public WallHeatTransferCoefficient3EqnBaseMaterial
{
public:
  WallHeatTransferCoefficient3EqnHeliumMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const MaterialProperty<Real> & _T_wall;
};

#endif /* WALLHEATTRANSFERCOEFFICIENT3EQNHELIUMMATERIAL_H */
