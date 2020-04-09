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
 * Kernel = energy_density * d(volumetric_strain)/dt
 * which is lumped to the nodes
 */
class PorousFlowHeatVolumetricExpansion : public TimeKernel
{
public:
  static InputParameters validParams();

  PorousFlowHeatVolumetricExpansion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Whether the Variable for this Kernel is a PorousFlow variable according to the Dictator
  const bool _var_is_porflow_var;

  /// Number of fluid phases
  const unsigned int _num_phases;

  /// Whether fluid is present
  const bool _fluid_present;

  /// Whether the porosity uses the volumetric strain at the closest quadpoint
  const bool _strain_at_nearest_qp;

  /// Porosity
  const MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;

  /// The nearest qp to the node
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// Nodal rock energy density
  const MaterialProperty<Real> & _rock_energy_nodal;

  /// d(nodal rock energy density)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _drock_energy_nodal_dvar;

  /// Nodal fluid density
  const MaterialProperty<std::vector<Real>> * const _fluid_density;

  /// d(nodal fluid density)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;

  /// Nodal fluid saturation
  const MaterialProperty<std::vector<Real>> * const _fluid_saturation_nodal;

  /// d(nodal fluid saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_saturation_nodal_dvar;

  /// Internal energy of the phases, evaluated at the nodes
  const MaterialProperty<std::vector<Real>> * const _energy_nodal;

  /// d(internal energy)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _denergy_nodal_dvar;

  /// Strain rate
  const MaterialProperty<Real> & _strain_rate_qp;

  /// d(strain rate)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dstrain_rate_qp_dvar;

  /**
   * Derivative of energy part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the energy part of the residual wrt this variable number
   */
  Real computedEnergyQpJac(unsigned int jvar);

  /**
   * Derivative of volumetric-strain part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the volumetric-strain part of the residual wrt this variable
   * number
   */
  Real computedVolQpJac(unsigned int jvar);
};
