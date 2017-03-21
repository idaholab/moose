/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWTEMPERATURE_H
#define POROUSFLOWTEMPERATURE_H

#include "DerivativeMaterialInterface.h"
#include "PorousFlowMaterial.h"

class PorousFlowTemperature;

template <>
InputParameters validParams<PorousFlowTemperature>();

/**
 * Creates temperature Materials
 */
class PorousFlowTemperature : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  PorousFlowTemperature(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Number of PorousFlow variables
  const unsigned int _num_pf_vars;

  /// Variable value of temperature at quadpoints or nodes
  const VariableValue & _temperature_var;

  /// Gradient(_temperature at quadpoints)
  const VariableGradient * const _grad_temperature_var;

  /// Whether the temperature coupled variable is a PorousFlow variable
  const bool _temperature_is_PF;

  /// the PorousFlow variable number of the temperature
  const unsigned int _t_var_num;

  /// Computed temperature at quadpoints or nodes
  MaterialProperty<Real> & _temperature;

  /// d(computed temperature)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> & _dtemperature_dvar;

  /// Grad(temperature) at the quadpoints (not needed for nodal_materials)
  MaterialProperty<RealGradient> * const _grad_temperature;

  /// d(grad temperature)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<Real>> * const _dgrad_temperature_dgradv;

  /// d(grad temperature)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<RealGradient>> * const _dgrad_temperature_dv;
};

#endif // POROUSFLOWTEMPERATURE_H
