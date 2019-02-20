#ifndef SPECIFICINTERFACIALAREABASEMATERIAL_H
#define SPECIFICINTERFACIALAREABASEMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"

class SpecificInterfacialAreaBaseMaterial;
class SpecificInterfacialAreaCurve;

template <>
InputParameters validParams<SpecificInterfacialAreaBaseMaterial>();

/**
 * Base class for computing specific interfacial area value for the 7-equation model
 */
class SpecificInterfacialAreaBaseMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  SpecificInterfacialAreaBaseMaterial(const InputParameters & parameters);

protected:
  const VariableValue & _alpha_vapor;
  const VariableValue & _rho_l;
  const VariableValue & _rho_v;
  const VariableValue & _T_l;
  const VariableValue & _T_v;
  const VariableValue & _p_l;
  const VariableValue & _p_v;
  const VariableValue & _vel_l;
  const VariableValue & _vel_v;
  const VariableValue & _D_h;
  const VariableValue & _arhou_l;
  const VariableValue & _arhou_v;

  const MaterialProperty<Real> & _alpha_liquid;
  const MaterialProperty<Real> & _dalpha_liquid_dbeta;
  // Fluid viscosities
  const MaterialProperty<Real> & _mu_l;
  const MaterialProperty<Real> & _mu_v;
  /// Surface tension
  const MaterialProperty<Real> & _surface_tension;

  /// Gravitational acceleration magnitude
  const Real & _gravity_magnitude;

  /// Interfacial area density
  MaterialProperty<Real> & _A_int;
  MaterialProperty<Real> & _dA_int_dbeta;
};

#endif // SPECIFICINTERFACIALAREABASEMATERIAL_H
