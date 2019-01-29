#ifndef WALLFRICTION3EQNBASEMATERIAL_H
#define WALLFRICTION3EQNBASEMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"

class WallFriction3EqnBaseMaterial;

template <>
InputParameters validParams<WallFriction3EqnBaseMaterial>();

/**
 * Base class for computing wall friction coefficient for 1-phase flow
 */
class WallFriction3EqnBaseMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  WallFriction3EqnBaseMaterial(const InputParameters & parameters);

protected:
  /// The wall friction coefficient
  const MaterialPropertyName _Cw_name;
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

#endif // WALLFRICTION3EQNBASEMATERIAL_H
