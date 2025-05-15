//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

/**
 * Adds mass diffusion for FlowChannelGasMix.
 */
class MassDiffusionBaseGasMixDGKernel : public ADDGKernel
{
public:
  static InputParameters validParams();

  MassDiffusionBaseGasMixDGKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  /**
   * Computes the flux that will be applied to both sides for a q-point.
   */
  virtual ADReal computeQpFlux() const = 0;

  /**
   * Computes the changes in position between elements and side
   *
   * @param[out] dx        Change in position of neighbor vs. element
   * @param[out] dx_side   Change in position of side vs. element
   */
  void computePositionChanges(Real & dx, Real & dx_side) const;

  /**
   * Linearly interpolates a quantity to the side position
   *
   * @param[in] y_elem    Value at the element
   * @param[in] y_neig    Value at the neighbor
   * @param[in] dx        Change in position of neighbor vs. element
   * @param[in] dx_side   Change in position of side vs. element
   */
  ADReal
  linearlyInterpolate(const ADReal & y_elem, const ADReal & y_neig, Real dx, Real dx_side) const;

  /**
   * Computes the gradient of a quantity in the channel direction
   *
   * @param[in] y_elem    Value at the element
   * @param[in] y_neig    Value at the neighbor
   * @param[in] dx        Change in position of neighbor vs. element
   */
  ADReal computeGradient(const ADReal & y_elem, const ADReal & y_neig, Real dx) const;

  /// Density for current element
  const ADMaterialProperty<Real> & _rho_elem;
  /// Density for neighbor element
  const ADMaterialProperty<Real> & _rho_neig;
  /// Diffusion coefficient for current element
  const ADMaterialProperty<Real> & _D_elem;
  /// Diffusion coefficient for neighbor element
  const ADMaterialProperty<Real> & _D_neig;
  /// Mass fraction for current element
  const ADMaterialProperty<Real> & _mass_fraction_elem;
  /// Mass fraction for neighbor element
  const ADMaterialProperty<Real> & _mass_fraction_neig;

  /// Cross-sectional area, linear
  const ADVariableValue & _A_linear;
  /// Flow channel direction
  const MaterialProperty<RealVectorValue> & _dir;
};
