/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWADVECTIVEFLUX_H
#define POROUSFLOWADVECTIVEFLUX_H

#include "Kernel.h"
#include "PorousFlowDictator.h"

class PorousFlowAdvectiveFlux;

template<>
InputParameters validParams<PorousFlowAdvectiveFlux>();

/**
 * Convective flux of component k in fluid phase alpha.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 */
class PorousFlowAdvectiveFlux : public Kernel
{
public:
  PorousFlowAdvectiveFlux(const InputParameters & parameters);

protected:
  virtual Real darcyQp(unsigned int ph);
  virtual Real darcyQpJacobian(unsigned int jvar, unsigned int ph);

  virtual Real computeQpResidual();
  virtual void computeResidual();
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

  /**
   * Full upwinding of both the residual and Jacobians.
   *
   * @param compute_res True if upwinding the residual, false otherwise
   * @param compute_jac Trus if upwiding the Jacobian, false otherwise
   * @param jvar PorousFlow variable to take derivatives wrt to
   */
  void upwind(bool compute_res, bool compute_jac, unsigned int jvar);

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;
  const MaterialProperty<std::vector<RealTensorValue> > & _dpermeability_dvar;
  /// Fluid density for each phase (at the node)
  const MaterialProperty<std::vector<Real> > & _fluid_density_node;
  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the node)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_density_node_dvar;
  /// Fluid density for each phase (at the qp)
  const MaterialProperty<std::vector<Real> > & _fluid_density_qp;
  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the qp)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_density_qp_dvar;
  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real> > & _fluid_viscosity;
  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_viscosity_dvar;
  /// Mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_fractions;
  /// Derivative of the mass fraction of each component in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _dmass_fractions_dvar;
  /// Nodal pore pressure in each phase
  const MaterialProperty<std::vector<Real> > & _pp;
  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient> > & _grad_p;
  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dgrad_p_dgrad_var;
  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real> > & _relative_permeability;
  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _drelative_permeability_dvar;

  /// PorousFlow UserObject
  const PorousFlowDictator & _porousflow_dictator_UO;

  /// Index of the component that this kernel acts on
  unsigned int _component_index;

  /// The number of fluid phases
  unsigned int _num_phases;

  /// Gravity. Defaults to 9.81 m/s^2
  RealVectorValue _gravity;
};

#endif // POROUSFLOWADVECTIVEFLUX_H
