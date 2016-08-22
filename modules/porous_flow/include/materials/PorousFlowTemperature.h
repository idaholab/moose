/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWTEMPERATURE_H
#define POROUSFLOWTEMPERATURE_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "PorousFlowDictator.h"

class PorousFlowTemperature;

template<>
InputParameters validParams<PorousFlowTemperature>();

/**
 * Creates temperature Materials
 */
class PorousFlowTemperature : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowTemperature(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /// The variable names UserObject for the PorousFlow variables
  const PorousFlowDictator & _dictator;

  /// Number of PorousFlow variables
  const unsigned int _num_pf_vars;

  /// Nodal value of temperature
  const VariableValue & _temperature_nodal_var;

  /// Quadpoint value of temperature
  const VariableValue & _temperature_qp_var;

  /// Gradient(_temperature at quadpoints)
  const VariableGradient & _grad_temperature;

  /// Whether the temperature coupled variable is a PorousFlow variable
  const bool _temperature_is_PF;

  /// the PorousFlow variable number of the temperature
  const unsigned int _t_var_num;

  /// Nearest node number for each quadpoint
  const MaterialProperty<unsigned int> & _node_number;

  /// Nodal value of temperature
  MaterialProperty<Real> & _temperature_nodal;

  /// Old value of nodal temperature
  MaterialProperty<Real> & _temperature_nodal_old;

  /// Quadpoint temperature
  MaterialProperty<Real> & _temperature_qp;

  /// Grad(temperature) at the quadpoints
  MaterialProperty<RealGradient> & _gradt_qp;

  /// d(nodal temperature)/d(nodal PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dtemperature_nodal_dvar;

  /// d(quadpoint temperature)/d(quadpoint PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dtemperature_qp_dvar;

  /// d(grad temperature)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<Real> > & _dgradt_qp_dgradv;

  /// d(grad temperature)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<RealGradient> > & _dgradt_qp_dv;

  const bool _yaqi_hacky;
};

#endif //POROUSFLOWTEMPERATURE_H
