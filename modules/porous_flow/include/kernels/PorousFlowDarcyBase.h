/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDARCYBASE_H
#define POROUSFLOWDARCYBASE_H

#include "Kernel.h"
#include "PorousFlowDictator.h"

class PorousFlowDarcyBase;

template <>
InputParameters validParams<PorousFlowDarcyBase>();

/**
 * Darcy advective flux.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 */
class PorousFlowDarcyBase : public Kernel
{
public:
  PorousFlowDarcyBase(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /// the Darcy part of the flux (this is the non-upwinded part)
  virtual Real darcyQp(unsigned int ph) const;

  /// Jacobian of the Darcy part of the flux
  virtual Real darcyQpJacobian(unsigned int jvar, unsigned int ph) const;

  /** The mobility of the fluid.  For multi-component Darcy flow
   * this is mass_fraction * fluid_density * relative_permeability / fluid_viscosity
   * @param nodenum The node-number to evaluate the mobility for
   * @param phase the fluid phase number
   */
  virtual Real mobility(unsigned nodenum, unsigned phase) const;

  /** The derivative of mobility with respect to PorousFlow variable pvar
   * @param nodenum The node-number to evaluate the mobility for
   * @param phase the fluid phase number
   * @param pvar the PorousFlow variable pvar
   */
  virtual Real dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const;

  enum JacRes
  {
    CALCULATE_RESIDUAL = 0,
    CALCULATE_JACOBIAN = 1
  };

  /**
   * Full upwinding of both the residual and Jacobians.
   *
   * If res_or_jac=CALCULATE_JACOBIAN then the residual
   * gets calculated anyway (becuase we have to know which
   * nodes are upwind and which are downwind) except we
   * don't actually need to store the residual in
   * _assembly.residualBlock and we don't need to save in save_in.
   *
   * If res_or_jac=CALCULATE_RESIDUAL then we don't have
   * to do all the Jacobian calculations.
   *
   * @param res_or_jac Whether to calculate the residual or the jacobian.
   * @param jvar PorousFlow variable to take derivatives wrt to
   */
  void upwind(JacRes res_or_jac, unsigned int jvar);

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeabiity)/d(porous-flow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_dvar;

  /// d(permeabiity)/d(grad(porous-flow variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dpermeability_dgradvar;

  /// Fluid density for each phase (at the node)
  const MaterialProperty<std::vector<Real>> & _fluid_density_node;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the node)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_node_dvar;

  /// Fluid density for each phase (at the qp)
  const MaterialProperty<std::vector<Real>> & _fluid_density_qp;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the qp)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_qp_dvar;

  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real>> & _fluid_viscosity;

  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_viscosity_dvar;

  /// Nodal pore pressure in each phase
  const MaterialProperty<std::vector<Real>> & _pp;

  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dgrad_p_dgrad_var;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<RealGradient>>> & _dgrad_p_dvar;

  /// PorousFlow UserObject
  const PorousFlowDictator & _porousflow_dictator;

  /// The number of fluid phases
  const unsigned int _num_phases;

  /// Gravity. Defaults to 9.81 m/s^2
  const RealVectorValue _gravity;
};

#endif // POROUSFLOWDARCYBASE_H
