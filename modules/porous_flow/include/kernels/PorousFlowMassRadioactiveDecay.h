//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivative.h"
#include "PorousFlowDictator.h"

/**
 * Kernel = _decay_rate * masscomponent
 * where mass_component =
 * porosity*sum_phases(density_phase*saturation_phase*massfrac_phase^component)
 * It is lumped to the nodes
 */
class PorousFlowMassRadioactiveDecay : public TimeKernel
{
public:
  static InputParameters validParams();

  PorousFlowMassRadioactiveDecay(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// The decay rate
  const Real _decay_rate;

  /// The fluid component index
  const unsigned int _fluid_component;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Whether the Variable for this Kernel is a PorousFlow variable according to the Dictator
  const bool _var_is_porflow_var;

  /// Number of fluid phases
  const unsigned int _num_phases;

  /// Whether the porosity uses the volumetric strain at the closest quadpoint
  const bool _strain_at_nearest_qp;

  /// Porosity at the nodes, but it can depend on grad(variables) which are actually evaluated at the qps
  const MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(PorousFlow variable) - these derivatives will be wrt variables at the nodes
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable) - remember these derivatives will be wrt grad(vars) at qps
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;

  /// The nearest qp to the node
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// Nodal fluid density
  const MaterialProperty<std::vector<Real>> & _fluid_density;

  /// d(nodal fluid density)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_dvar;

  /// Nodal fluid saturation
  const MaterialProperty<std::vector<Real>> & _fluid_saturation_nodal;

  /// d(nodal fluid saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_saturation_nodal_dvar;

  /// Nodal mass fraction
  const MaterialProperty<std::vector<std::vector<Real>>> & _mass_frac;

  /// d(nodal mass fraction)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_frac_dvar;

  /**
   * Derivative of residual with respect to PorousFlow variable number pvar
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param pvar take the derivative of the residual wrt this PorousFlow variable
   */
  Real computeQpJac(unsigned int pvar);
};
