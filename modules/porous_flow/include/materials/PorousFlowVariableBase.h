/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWVARIABLEBASE_H
#define POROUSFLOWVARIABLEBASE_H

#include "DerivativeMaterialInterface.h"
#include "PorousFlowMaterial.h"

class PorousFlowVariableBase;

template <>
InputParameters validParams<PorousFlowVariableBase>();

/**
 * Base class for thermophysical variable materials, which assemble materials for
 * primary variables such as porepressure and saturation at the nodes
 * and quadpoints for all phases as required
 */
class PorousFlowVariableBase : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  PorousFlowVariableBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Number of phases
  const unsigned int _num_phases;

  /// Number of components
  const unsigned int _num_components;

  /// Number of PorousFlow variables
  const unsigned int _num_pf_vars;

  /// Computed nodal or quadpoint values of porepressure of the phases
  MaterialProperty<std::vector<Real>> & _porepressure;

  /// d(porepressure)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real>>> & _dporepressure_dvar;

  /// Grad(p) at the quadpoints
  MaterialProperty<std::vector<RealGradient>> * const _gradp_qp;

  /// d(grad porepressure)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<Real>>> * const _dgradp_qp_dgradv;

  /// d(grad porepressure)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<RealGradient>>> * const _dgradp_qp_dv;

  /// Computed nodal or qp saturation of the phases
  MaterialProperty<std::vector<Real>> & _saturation;

  /// d(saturation)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real>>> & _dsaturation_dvar;

  /// Grad(s) at the quadpoints
  MaterialProperty<std::vector<RealGradient>> * const _grads_qp;

  /// d(grad saturation)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<Real>>> * const _dgrads_qp_dgradv;

  /// d(grad saturation)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<RealGradient>>> * const _dgrads_qp_dv;
};

#endif // POROUSFLOWVARIABLEBASE_H
