//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowLineGeometry.h"

class PorousFlowDictator;
class PorousFlowSumQuantity;
class Function;

/**
 * Peaceman borehole for finite-volume PorousFlow simulations, controlled either by the bottomhole
 * pressure (control = bhp, as PorousFlowPeacemanBorehole) or by the total mass rate
 * (control = rate).  With control = rate the bottomhole pressure
 * is a scalar variable, solved fully coupled with the PorousFlow variables.
 *
 * As for the finite-element PorousFlowPeacemanBorehole, there is one DiracKernel per fluid phase
 * and fluid component, and at borehole point i the flux of this DiracKernel is
 *   |character| * WI_i * M * (p_phase - p_conn),   p_conn = p_bot + unit_weight * (x_i - x_bottom)
 * where M is the product of the multipliers selected by use_mobility, use_relative_permeability,
 * mass_fraction_component and multiplying_var.  Heat transfer (the enthalpy and internal-energy
 * multipliers and function_of = temperature of PorousFlowPeacemanBorehole) is not implemented.
 *
 * Two options change the flow direction and injection:
 *  - allow_crossflow = true lets fluid flow in either direction at every point (otherwise the sign
 *    of character makes the borehole a producer or an injector, as in PorousFlowPeacemanBorehole)
 *  - with injection_phase given, the flow direction at a point is decided by the total volumetric
 *    flow, and at injecting points only the injection phase enters, with the total mobility of the
 *    cell
 */
class FVPorousFlowPeacemanBorehole : public PorousFlowLineGeometry
{
public:
  static InputParameters validParams();

  FVPorousFlowPeacemanBorehole(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void addPoints() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeResidualAndJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /// Whether the borehole is rate controlled
  bool rateControlled() const { return _control == Control::rate; }

  /// Number of the bottomhole-pressure scalar variable (rate control only)
  unsigned int bottomPressureVariableNumber() const;

protected:
  /// Not used: the residuals are assembled from cellOutflow()
  virtual Real computeQpResidual() override;

  /// Flux out of the porous medium (positive) at the current Dirac point, which is borehole point i
  ADReal pointOutflow(unsigned int i);

  /**
   * Sums over the borehole points in the current element: the outflow of this DiracKernel, and
   * the contribution to the residual of the bottomhole-pressure equation (rate control only)
   */
  void cellOutflow(ADReal & outflow, ADReal & rate_residual);

  /// Peaceman well constant of borehole point i, which lies in the current element
  Real wellConstant(unsigned int i, const RealTensorValue & perm) const;

  /// UserObject that holds information (number of phases, components, etc)
  const PorousFlowDictator & _dictator;

  /// Total outflow recorded every residual evaluation (optional)
  PorousFlowSumQuantity * const _total_outflow_mass;

  /// The fluid phase whose pressure (and mobility, etc) controls the flux
  const unsigned int _ph;

  /// Whether the flux is multiplied by the mass fraction of _sp in _ph
  const bool _use_mass_fraction;

  /// The fluid component for the mass fraction multiplier
  const unsigned int _sp;

  /// Multiply the flux by the relative permeability
  const bool _use_relative_permeability;

  /// Multiply the flux by the mobility (density * relative permeability / viscosity)
  const bool _use_mobility;

  /// Whether the injection phase alone enters at injecting points, with the total mobility
  const bool _use_injection_phase;

  /// The phase injected at injecting points (when _use_injection_phase)
  const unsigned int _injection_phase;

  /// Whether fluid may flow in either direction at every point
  const bool _allow_crossflow;

  /// The flux is multiplied by |character|, and its sign selects production or injection
  const Function & _character;

  /// (fluid_density * gravity) as a vector pointing downwards
  const RealVectorValue _unit_weight;

  /// Dimensionless constant used in evaluating the borehole effective radius
  const Real _re_constant;

  /// If positive, this is used instead of the computed well constant
  const Real _well_constant;

  /// Borehole control
  enum class Control
  {
    bhp,
    rate
  } _control;

  /// Bottomhole pressure (control = bhp)
  const Function * const _bottom_p_function;

  /// Prescribed total mass rate, positive for production (control = rate)
  const Function * const _rate;

  /// The bottomhole-pressure variable (control = rate)
  const MooseVariableScalar * const _bottom_p_var;

  /// The bottomhole pressure (control = rate)
  const ADVariableValue * const _bottom_p_value;

  /// Whether this DiracKernel adds the prescribed rate to the bottomhole-pressure equation
  bool _adds_rate;

  /// Fluxes are multiplied by this variable
  const ADVariableValue & _multiplying_var;

  /// Phase pressures
  const ADMaterialProperty<std::vector<Real>> & _pressure;

  /// Mass fraction of each component in each phase
  const ADMaterialProperty<std::vector<std::vector<Real>>> * const _mass_fractions;

  /// Fluid density
  const ADMaterialProperty<std::vector<Real>> & _density;

  /// Fluid viscosity
  const ADMaterialProperty<std::vector<Real>> & _viscosity;

  /// Relative permeability
  const ADMaterialProperty<std::vector<Real>> & _relperm;

  /// Permeability
  const ADMaterialProperty<RealTensorValue> & _permeability;

  /// Rotation matrix taking the direction of segment i to the z axis
  std::vector<RealTensorValue> _rot_matrix;
};
