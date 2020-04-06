#pragma once

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"

class SinglePhaseFluidProperties;

/**
 * Computes Reynolds number as a material property
 */
class ReynoldsNumberMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  ReynoldsNumberMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Reynolds number property name
  const MaterialPropertyName & _Re_name;

  /// Density of the phase
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> * const _drho_dbeta;
  const MaterialProperty<Real> & _drho_darhoA;

  /// Velocity of the phase
  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;

  /// Dynamic viscosity of the phase
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> * const _dmu_dbeta;
  const MaterialProperty<Real> & _dmu_darhoA;
  const MaterialProperty<Real> & _dmu_darhouA;
  const MaterialProperty<Real> & _dmu_darhoEA;

  /// Reynolds
  MaterialProperty<Real> & _Re;
  MaterialProperty<Real> * const _dRe_dbeta;
  MaterialProperty<Real> & _dRe_darhoA;
  MaterialProperty<Real> & _dRe_darhouA;
  MaterialProperty<Real> & _dRe_darhoEA;

public:
  static InputParameters validParams();
};
