//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialBase.h"

/**
 * Base class for PorousFlow relative permeability materials. All materials
 * that derive from this class must override relativePermeability() and
 * dRelativePermeability()
 */
template <bool is_ad>
class PorousFlowRelativePermeabilityBaseTempl : public PorousFlowMaterialBase
{
public:
  static InputParameters validParams();

  PorousFlowRelativePermeabilityBaseTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /**
   * Effective saturation of fluid phase
   * @param saturation true saturation
   * @return effective saturation
   */
  virtual GenericReal<is_ad> effectiveSaturation(GenericReal<is_ad> saturation) const;

  /**
   * Relative permeability equation (must be overriden in derived class)
   * @param seff effective saturation
   * @return relative permeability
   */
  virtual GenericReal<is_ad> relativePermeability(GenericReal<is_ad> seff) const = 0;

  /**
   * Derivative of relative permeability with respect to effective saturation
   * @param seff effective saturation
   * @return derivative of relative permeability wrt effective saturation
   */
  virtual Real dRelativePermeability(Real seff) const = 0;

  /// Relative permeability is multiplied by this quantity
  const Real _scaling;

  /// Saturation material property
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _saturation;

  /// Relative permeability material property
  GenericMaterialProperty<Real, is_ad> & _relative_permeability;

  /// Derivative of relative permeability wrt phase saturation
  MaterialProperty<Real> * const _drelative_permeability_ds;

  /// Residual saturation of specified phase
  const Real _s_res;

  /// Sum of residual saturations over all phases
  const Real _sum_s_res;

  /// Derivative of effective saturation with respect to saturation
  const Real _dseff_ds;
};

typedef PorousFlowRelativePermeabilityBaseTempl<false> PorousFlowRelativePermeabilityBase;
typedef PorousFlowRelativePermeabilityBaseTempl<true> ADPorousFlowRelativePermeabilityBase;
