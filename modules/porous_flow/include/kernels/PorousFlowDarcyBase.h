//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "PorousFlowDictator.h"

/**
 * Darcy advective flux.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 * Alternately, after some number of upwind-downwind swaps,
 * another type of handling of the mobility may be employed
 * (see fallback_scheme, below)
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowDarcyBaseTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowDarcyBaseTempl(const InputParameters & parameters);

protected:
  virtual void timestepSetup() override;
  virtual void jacobianSetup() override;
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /// The Darcy part of the flux (this is the non-upwinded part)
  virtual GenericReal<is_ad> darcyQp(unsigned int ph) const;

  /// Jacobian of the Darcy part of the flux -- non-AD path only
  virtual Real darcyQpJacobian(unsigned int jvar, unsigned int ph) const;

  /**
   * The mobility of the fluid.  For multi-component Darcy flow
   * this is mass_fraction * fluid_density * relative_permeability / fluid_viscosity
   * @param nodenum The node-number to evaluate the mobility for
   * @param phase the fluid phase number
   */
  virtual GenericReal<is_ad> mobility(unsigned nodenum, unsigned phase) const;

  /**
   * The derivative of mobility with respect to PorousFlow variable pvar -- non-AD path only
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
   * Computation of the residual and Jacobian (non-AD path only).
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
  const GenericMaterialProperty<RealTensorValue, is_ad> & _permeability;

  /// d(permeabiity)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<RealTensorValue>> * const _dpermeability_dvar;

  /// d(permeabiity)/d(grad(PorousFlow variable)) -- null for AD path
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> * const _dpermeability_dgradvar;

  /// Fluid density for each phase (at the node)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_density_node;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the node) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_node_dvar;

  /// Fluid density for each phase (at the qp)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_density_qp;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the qp) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_qp_dvar;

  /// Viscosity of each component in each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_viscosity;

  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_viscosity_dvar;

  /// Nodal pore pressure in each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _pp;

  /// Gradient of the pore pressure in each phase
  const GenericMaterialProperty<std::vector<RealGradient>, is_ad> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dgrad_p_dgrad_var;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<RealGradient>>> * const _dgrad_p_dvar;

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
  std::vector<std::vector<GenericReal<is_ad>>> _proto_flux;

  /**
   * Derivative of _proto_flux with respect to nodal variables.
   * This gets modified with the mobility and its derivative during
   * computation of MOOSE's Jacobian. Only used on the non-AD path.
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
   * @param res_or_jac whether to compute the residual or jacobian (non-AD: CALCULATE_JACOBIAN
   *   fills _jacobian; AD: always pass CALCULATE_RESIDUAL)
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

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;

private:
  /// Build proto fluxes (without mobility weighting) for all phases and nodes
  void computeProtoFluxWithoutMobility();

  /// Ensure per-element upwind/downwind counters are allocated for this element
  void initializeUpwindTracking(unsigned elem, unsigned int num_nodes);

  /// Update upwind/downwind counters from the sign of _proto_flux
  void updateUpwindCounts(unsigned elem, unsigned int num_nodes);

  /// Compute per-phase maximum upwind/downwind swap counts for this element
  std::vector<unsigned> computeMaxSwaps(unsigned elem, unsigned int num_nodes) const;

  /// Apply selected upwinding/fallback scheme for all phases
  void
  applyUpwinding(const std::vector<unsigned> & max_swaps, JacRes res_or_jac, unsigned int pvar);

  /**
   * For the AD path: fills _proto_flux (per-phase, per-node ADReal) by integrating
   * darcyQp and applying the upwinding scheme with GenericReal<is_ad> mobility.
   * Also optionally records upwind/downwind counts for the fallback-scheme heuristic.
   * @param do_counting whether to update _num_upwinds/_num_downwinds
   */
  void adComputeProtoFlux(bool do_counting);

  /**
   * For the AD path: performs the proto-flux/upwinding pass and hands the per-node
   * ADReal residuals to addJacobianWithoutConstraints, which extracts every Jacobian
   * block (diagonal and off-diagonal) from the ADReal derivatives in a single pass.
   */
  void adComputeJacobian();

  /// Assemble the real-valued residual vector from _proto_flux into the tagged local residual
  /// (and apply save_in). Shared by the AD and non-AD residual paths.
  void assembleProtoFluxResidual();

  /// Element pointer cached on the first AD computeOffDiagJacobian() call for an element so
  /// that the remaining off-diagonal calls (one per coupled variable) become no-ops; the
  /// single AD pass already filled all blocks. Reset each Jacobian evaluation in jacobianSetup().
  const Elem * _my_elem_darcy = nullptr;
};

typedef PorousFlowDarcyBaseTempl<false> PorousFlowDarcyBase;
typedef PorousFlowDarcyBaseTempl<true> ADPorousFlowDarcyBase;
