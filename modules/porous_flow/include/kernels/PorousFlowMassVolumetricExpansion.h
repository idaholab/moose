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
#include "RankTwoTensor.h"

/**
 * Kernel = mass_component * d(volumetric_strain)/dt
 * where mass_component =
 * porosity*sum_phases(density_phase*saturation_phase*massfrac_phase^component)
 * which is lumped to the nodes
 * If _multiply_by_density = false then density_phase does not appear in the above expression
 */
class PorousFlowMassVolumetricExpansion : public TimeKernel
{
public:
  static InputParameters validParams();

  PorousFlowMassVolumetricExpansion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// The fluid component index
  const unsigned int _fluid_component;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Whether the Variable for this Kernel is a PorousFlow variable according to the Dictator
  const bool _var_is_porflow_var;

  /// Number of displacement variables
  unsigned int _ndisp;

  /// Variable number of the displacements variables
  std::vector<unsigned int> _disp_var_num;

  /// Number of fluid phases
  const unsigned int _num_phases;

  /// Whether the porosity uses the volumetric strain at the closest quadpoint
  const bool _strain_at_nearest_qp;

  /// Whether to multiply by density: if true then this Kernel involves the fluid mass, otherwise it involves the fluid volume
  const bool _multiply_by_density;

  /// Porosity
  const MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;

  /// The nearest qp to the node
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// Fluid density
  const MaterialProperty<std::vector<Real>> * const _fluid_density;

  /// d(fluid density)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;

  /// Fluid saturation
  const MaterialProperty<std::vector<Real>> & _fluid_saturation;

  /// d(fluid saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_saturation_dvar;

  /// Mass fraction
  const MaterialProperty<std::vector<std::vector<Real>>> & _mass_frac;

  /// d(mass fraction)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_frac_dvar;

  /// Strain rate
  const MaterialProperty<Real> & _strain_rate_qp;

  /// d(strain rate)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dstrain_rate_qp_dvar;

  /**
   * Derivative of mass part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the mass part of the residual wrt this variable number
   */
  Real computedMassQpJac(unsigned int jvar) const;

  /**
   * Derivative of volumetric-strain part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the volumetric-strain part of the residual wrt this variable
   * number
   */
  Real computedVolQpJac(unsigned int jvar) const;
};
