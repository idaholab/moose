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

template<>
InputParameters validParams<PorousFlowTemperature>();

/**
 * Creates temperature Materials
 */
class PorousFlowTemperature : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  PorousFlowTemperature(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Number of PorousFlow variables
  const unsigned int _num_pf_vars;

  /// Quadpoint value of temperature
  const VariableValue & _temperature_qp_var;

  /// Gradient(_temperature at quadpoints)
  const VariableGradient & _grad_temperature;

  /// Whether the temperature coupled variable is a PorousFlow variable
  const bool _temperature_is_PF;

  /// the PorousFlow variable number of the temperature
  const unsigned int _t_var_num;

  /// Quadpoint temperature
  MaterialProperty<Real> & _temperature_qp;

  /// Grad(temperature) at the quadpoints
  MaterialProperty<RealGradient> & _gradt_qp;

  /// d(quadpoint temperature)/d(quadpoint PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dtemperature_qp_dvar;

  /// d(grad temperature)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<Real> > & _dgradt_qp_dgradv;

  /// d(grad temperature)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<RealGradient> > & _dgradt_qp_dv;
};

#endif //POROUSFLOWTEMPERATURE_H
