//
// This file is part of Sockeye heat pipe performance code.
// All rights reserved; see COPYRIGHT for full restrictions.
//

#pragma once

#include "MassDiffusionBaseGasMixDGKernel.h"

class VaporMixtureFluidProperties;
class SinglePhaseFluidProperties;

/**
 * Adds mass diffusion to the energy equation for FlowChannelGasMix.
 */
class MassDiffusionEnergyGasMixDGKernel : public MassDiffusionBaseGasMixDGKernel
{
public:
  static InputParameters validParams();

  MassDiffusionEnergyGasMixDGKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpFlux() const override;

  /**
   * Components the total enthalpy for a mixture component
   *
   * @param[in] p     Component (partial) pressure
   * @param[in] T     Component/mixture temperature
   * @param[in] vel   Component velocity
   * @param[in] fp    Component fluid properties
   */
  ADReal computeComponentTotalEnthalpy(const ADReal & p,
                                       const ADReal & T,
                                       const ADReal & vel,
                                       const SinglePhaseFluidProperties & fp) const;

  /// Mixture pressure for current element
  const ADMaterialProperty<Real> & _p_elem;
  /// Mixture pressure for neighbor element
  const ADMaterialProperty<Real> & _p_neig;

  /// Temperature for current element
  const ADMaterialProperty<Real> & _T_elem;
  /// Temperature for neighbor element
  const ADMaterialProperty<Real> & _T_neig;

  /// Mixture velocity for current element
  const ADMaterialProperty<Real> & _vel_elem;
  /// Mixture velocity for neighbor element
  const ADMaterialProperty<Real> & _vel_neig;

  /// Fluid properties
  const VaporMixtureFluidProperties & _fp;
  const SinglePhaseFluidProperties & _fp_primary;
  const SinglePhaseFluidProperties & _fp_secondary;
};
