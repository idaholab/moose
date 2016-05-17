/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWVARIABLEBASE_H
#define POROUSFLOWVARIABLEBASE_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "PorousFlowDictator.h"

class PorousFlowVariableBase;

template<>
InputParameters validParams<PorousFlowVariableBase>();

/**
 * Base class for thermophysical variable materials, which assemble materials for
 * primary variables such as porepressure, saturation, temperature, etc, at the nodes
 * and quadpoints for all phases as required
 */
class PorousFlowVariableBase : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowVariableBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /// The variable names UserObject for the PorousFlow variables
  const PorousFlowDictator & _dictator;

  /// Number of phases
  const unsigned int _num_phases;

  /// Number of components
  const unsigned int _num_components;

  /// Number of PorousFlow variables
  const unsigned int _num_pf_vars;

  /// Nodal value of temperature
  const VariableValue & _temperature_nodal_var;

  /// Quadpoint value of temperature
  const VariableValue & _temperature_qp_var;

  /// Moose variable number of the phase0 temperature
  const unsigned int _temperature_varnum;

  /// Nodal values of porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_nodal;

  /// Old values of nodal porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_nodal_old;

  /// Quadpoint porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_qp;

  /// Grad(p) at the quadpoints
  MaterialProperty<std::vector<RealGradient> > & _gradp_qp;

  /// d(nodal porepressure)/d(nodal PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_nodal_dvar;

  /// d(quadpoint porepressure)/d(quadpoint PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_qp_dvar;

  /// d(grad porepressure)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<Real> > > & _dgradp_qp_dgradv;

  /// d(grad porepressure)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<RealGradient> > > & _dgradp_qp_dv;

  /// Nodal saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation_nodal;

  /// Old value of nodal saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation_nodal_old;

  /// Quadpoint saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation_qp;

  /// Grad(s) at the quadpoints
  MaterialProperty<std::vector<RealGradient> > & _grads_qp;

  /// d(nodal saturation)/d(nodal PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_nodal_dvar;

  /// d(quadpoint saturation)/d(quadpoint porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_qp_dvar;

  /// d(grad saturation)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<Real> > > & _dgrads_qp_dgradv;

  /// d(grad saturation)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<RealGradient> > > & _dgrads_qp_dv;

  /// Nodal values of the temperature of the phases
  MaterialProperty<std::vector<Real> > & _temperature_nodal;

  /// Quadpoint values of the temperature of the phases
  MaterialProperty<std::vector<Real> > & _temperature_qp;

  /// d(nodal temperature)/d(nodal PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dtemperature_nodal_dvar;

  /// d(quadpoint temperature)/d(quadpoint PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dtemperature_qp_dvar;
};

#endif //POROUSFLOWVARIABLEBASE_H
