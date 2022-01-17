#pragma once

#include "ADKernel.h"

/**
 * Computes wall friction term for single phase flow
 *
 * See RELAP-7 Theory Manual, pg. 71, Equation (230) {eq:wall_friction_force_2phase}
 */
class ADOneD3EqnMomentumFriction : public ADKernel
{
public:
  ADOneD3EqnMomentumFriction(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// area
  const ADVariableValue & _A;

  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;

  /// Density
  const ADMaterialProperty<Real> & _rho;

  /// velocity
  const ADMaterialProperty<Real> & _vel;

  /// Darcy friction factor
  const ADMaterialProperty<Real> & _f_D;

public:
  static InputParameters validParams();
};
