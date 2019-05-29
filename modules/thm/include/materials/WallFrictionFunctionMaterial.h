#pragma once

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"

class WallFrictionFunctionMaterial;
class Function;

template <>
InputParameters validParams<WallFrictionFunctionMaterial>();

/**
 * Converts Darcy friction factor function into material property
 */
class WallFrictionFunctionMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  WallFrictionFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Function & _function;

  const MaterialPropertyName _f_D_name;
  MaterialProperty<Real> & _f_D;
  MaterialProperty<Real> * const _df_D_dbeta;
  MaterialProperty<Real> & _df_D_darhoA;
  MaterialProperty<Real> & _df_D_darhouA;
  MaterialProperty<Real> & _df_D_darhoEA;
};
