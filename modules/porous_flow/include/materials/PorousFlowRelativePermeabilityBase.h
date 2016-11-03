/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYBASE_H
#define POROUSFLOWRELATIVEPERMEABILITYBASE_H

#include "PorousFlowMaterialBase.h"

class PorousFlowRelativePermeabilityBase;

template<>
InputParameters validParams<PorousFlowRelativePermeabilityBase>();

/**
 * Base class for PorousFlow relative permeability materials. All materials
 * that derive from this class must override relativePermeability() and
 * dRelativePermeability_dS() (and optionally effectiveSaturation())
 */
class PorousFlowRelativePermeabilityBase : public PorousFlowMaterialBase
{
public:
  PorousFlowRelativePermeabilityBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /**
   * Effective saturation of fluid phase
   * @param saturation real saturation
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
   * Derivative of relative permeability equation (must be overriden in derived class)
   * @param seff effective saturation
   * @return derivative of relative permeability wrt saturation
   */
  virtual Real dRelativePermeability_dS(Real seff) const = 0;

  /// Name of (dummy) saturation primary variable
  VariableName _saturation_variable_name;

  /// Saturation material property
  const MaterialProperty<std::vector<Real> > & _saturation_nodal;

  /// Relative permeability material property
  MaterialProperty<Real> & _relative_permeability;

  /// Derivative of relative permeability wrt phase saturation
  MaterialProperty<Real> & _drelative_permeability_ds;

  /// Residual saturation of specified phase
  const Real _s_res;

  /// Sum of residual saturations over all phases
  const Real _sum_s_res;
};

#endif //POROUSFLOWRELATIVEPERMEABILITYBASE_H
