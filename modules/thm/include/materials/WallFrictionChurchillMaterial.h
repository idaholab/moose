#ifndef WALLFRICTIONCHURCHILLMATERIAL_H
#define WALLFRICTIONCHURCHILLMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterfaceRelap.h"

class WallFrictionChurchillMaterial;

template <>
InputParameters validParams<WallFrictionChurchillMaterial>();

/**
 * Computes drag coefficient using the Churchill formula for Fanning friction factor
 */
class WallFrictionChurchillMaterial : public DerivativeMaterialInterfaceRelap<Material>
{
public:
  WallFrictionChurchillMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The wall friction coefficient
  MaterialProperty<Real> & _Cw;
  MaterialProperty<Real> & _dCw_drhoA;
  MaterialProperty<Real> & _dCw_drhouA;
  MaterialProperty<Real> & _dCw_drhoEA;
  const MaterialProperty<Real> & _mu;

  /// Density of the phase
  const VariableValue & _rho;
  /// Velocity (x-component)
  const VariableValue & _vel;
  /// Hydraulic diameter
  const VariableValue & _D_h;
  /// Roughness of the surface
  const Real & _roughness;
};

#endif // WALLFRICTIONCHURCHILLMATERIAL_H
