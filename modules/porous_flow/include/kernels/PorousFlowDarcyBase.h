//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "PorousFlowDictator.h"

/**
 * Darcy advective flux.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 * Alternately, after some number of upwind-downwind swaps,
 * another type of handling of the mobility may be employed
 * (see fallback_scheme, below)
 */
class PorousFlowDarcyBase : public Kernel
{
public:
  static InputParameters validParams();

  PorousFlowDarcyBase(const InputParameters & parameters);

protected:
  virtual void timestepSetup() override;
  virtual Real computeQpResidual() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /// The Darcy part of the flux (this is the non-upwinded part)
  virtual Real darcyQp(unsigned int ph) const;

  /// Jacobian of the Darcy part of the flux
  virtual Real darcyQpJacobian(unsigned int jvar, unsigned int ph) const;

  /**
   * The mobility of the fluid.  For multi-component Darcy flow
   * this is mass_fraction * fluid_density * relative_permeability / fluid_viscosity
   * @param nodenum The node-number to evaluate the mobility for
   * @param phase the fluid phase number
   */
  virtual Real mobility(unsigned nodenum, unsigned phase) const;

  /**
   * The derivative of mobility with respect to PorousFlow variable pvar
   * @param nodenum The node-number to evaluate the mobility for
   * @param phase the fluid phase number
   * @param pvar the PorousFlow variable pvar
   */
  virtual Real dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const;

  enum class JacRes
  {
    CALCULATE_RESIDUAL = 0,
    CALCULATE_JACOBIAN = 1
  };

  using ResidualObject::computeResidualAndJacobian;

  /**
   * Computation of the residual and Jacobian.
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
  void computeResidualAndJacobian(JacRes res_or_jac, unsigned int jvar);

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeabiity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_dvar;

  /// d(permeabiity)/d(grad(PorousFlow variable))
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

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// The number of fluid phases
  const unsigned int _num_phases;

  /// Gravity. Defaults to 9.81 m/s^2
  const RealVectorValue _gravity;

  /// Flag to check whether permeabiity derivatives are non-zero
  const bool _perm_derivs;

  /**
   * If the number of upwind-downwind swaps is less than this amount then
   * full upwinding is used.  Otherwise the fallback scheme is employed
   */
  const unsigned _full_upwind_threshold;

  /**
   * If full upwinding is failing due to nodes swapping between upwind
   * and downwind in successive nonlinear iterations (within one timestep
   * and element) a fallback scheme is used instead.
   * QUICK: use the nodal mobilities, as in full upwinding, but do not
   * attempt to conserve fluid mass.
   * HARMONIC: assign the harmonic mean of the mobilities to the entire
   * element.
   */
  const enum class FallbackEnum { QUICK, HARMONIC } _fallback_scheme;

  /**
   * The Darcy flux.  When multiplied by the mobility, this is the
   * mass flux of fluid moving out of a node.  This multiplication occurs
   * during the formation of the residual and Jacobian.
   * _proto_flux[num_phases][num_nodes]
   */
  std::vector<std::vector<Real>> _proto_flux;

  /**
   * Derivative of _proto_flux with respect to nodal variables.
   * This gets modified with the mobility and its derivative during
   * computation of MOOSE's Jacobian
   */
  std::vector<std::vector<std::vector<Real>>> _jacobian;

  /**
   * Number of nonlinear iterations (in this timestep and this element)
   * that a node is an upwind node for a given fluid phase.
   * _num_upwinds[element_number][phase][node_number_in_element]
   */
  std::unordered_map<unsigned, std::vector<std::vector<unsigned>>> _num_upwinds;

  /**
   * Number of nonlinear iterations (in this timestep and this element)
   * that a node is an downwind node for a given fluid phase.
   * _num_upwinds[element_number][phase][node_number_in_element]
   */
  std::unordered_map<unsigned, std::vector<std::vector<unsigned>>> _num_downwinds;

  /**
   * Calculate the residual or Jacobian using full upwinding
   * @param res_or_jac whether to compute the residual or jacobian
   * @param ph fluid phase number
   * @param pvar differentiate wrt to this PorousFlow variable (when computing the jacobian)
   */
  void fullyUpwind(JacRes res_or_jac, unsigned int ph, unsigned int pvar);

  /**
   * Calculate the residual or Jacobian using the nodal mobilities,
   * but without conserving fluid mass.
   * @param res_or_jac whether to compute the residual or jacobian
   * @param ph fluid phase number
   * @param pvar differentiate wrt to this PorousFlow variable (when computing the jacobian)
   */
  void quickUpwind(JacRes res_or_jac, unsigned int ph, unsigned int pvar);

  /**
   * Calculate the residual or Jacobian by using the harmonic mean
   * of the nodal mobilities for the entire element
   * @param res_or_jac whether to compute the residual or jacobian
   * @param ph fluid phase number
   * @param pvar differentiate wrt to this PorousFlow variable (when computing the jacobian)
   */
  void harmonicMean(JacRes res_or_jac, unsigned int ph, unsigned int pvar);
};
