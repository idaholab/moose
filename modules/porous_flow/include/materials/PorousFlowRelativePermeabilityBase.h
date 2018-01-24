//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWRELATIVEPERMEABILITYBASE_H
#define POROUSFLOWRELATIVEPERMEABILITYBASE_H

#include "PorousFlowMaterialBase.h"

class PorousFlowRelativePermeabilityBase;

template <>
InputParameters validParams<PorousFlowRelativePermeabilityBase>();

/**
 * Base class for PorousFlow relative permeability materials. All materials
 * that derive from this class must override relativePermeability() and
 * dRelativePermeability()
 */
class PorousFlowRelativePermeabilityBase : public PorousFlowMaterialBase
{
public:
  PorousFlowRelativePermeabilityBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /**
   * Effective saturation of fluid phase
   * @param saturation true saturation
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real saturation) const;

  /**
   * Relative permeability equation (must be overriden in derived class)
   * @param seff effective saturation
   * @return relative permeability
   */
  virtual Real relativePermeability(Real seff) const = 0;

  /**
   * Derivative of relative permeability with respect to effective saturation
   * @param seff effective saturation
   * @return derivative of relative permeability wrt effective saturation
   */
  virtual Real dRelativePermeability(Real seff) const = 0;

  /// Relative permeability is multiplied by this quantity
  const Real _scaling;

  /// Name of (dummy) saturation primary variable
  VariableName _saturation_variable_name;

  /// Saturation material property
  const MaterialProperty<std::vector<Real>> & _saturation;

  /// Relative permeability material property
  MaterialProperty<Real> & _relative_permeability;

  /// Derivative of relative permeability wrt phase saturation
  MaterialProperty<Real> & _drelative_permeability_ds;

  /// Residual saturation of specified phase
  const Real _s_res;

  /// Sum of residual saturations over all phases
  const Real _sum_s_res;

  /// Derivative of effective saturation with respect to saturation
  const Real _dseff_ds;
};

#endif // POROUSFLOWRELATIVEPERMEABILITYBASE_H
