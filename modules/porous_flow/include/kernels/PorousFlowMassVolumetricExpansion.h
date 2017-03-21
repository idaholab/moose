/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMASSVOLUMETRICEXPANSION_H
#define POROUSFLOWMASSVOLUMETRICEXPANSION_H

#include "TimeDerivative.h"
#include "PorousFlowDictator.h"
#include "RankTwoTensor.h"

// Forward Declarations
class PorousFlowMassVolumetricExpansion;

template <>
InputParameters validParams<PorousFlowMassVolumetricExpansion>();

/**
 * Kernel = mass_component * d(volumetric_strain)/dt
 * where mass_component =
 * porosity*sum_phases(density_phase*saturation_phase*massfrac_phase^component)
 * which is lumped to the nodes
 */
class PorousFlowMassVolumetricExpansion : public TimeKernel
{
public:
  PorousFlowMassVolumetricExpansion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// the fluid component index
  const unsigned int _fluid_component;

  /// holds info on the Porous Flow variables
  const PorousFlowDictator & _dictator;

  /// whether the Variable for this Kernel is a porous-flow variable according to the Dictator
  const bool _var_is_porflow_var;

  /// number of displacement variables
  unsigned int _ndisp;

  /// variable number of the displacements variables
  std::vector<unsigned int> _disp_var_num;

  /// whether the porosity uses the volumetric strain at the closest quadpoint
  const bool _strain_at_nearest_qp;

  /// porosity
  const MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(porous-flow variable)
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad porous-flow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;

  /// the nearest qp to the node
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// fluid density
  const MaterialProperty<std::vector<Real>> & _fluid_density;

  /// d(fluid density)/d(porous-flow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_dvar;

  /// fluid saturation
  const MaterialProperty<std::vector<Real>> & _fluid_saturation;

  /// d(fluid saturation)/d(porous-flow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_saturation_dvar;

  /// mass fraction
  const MaterialProperty<std::vector<std::vector<Real>>> & _mass_frac;

  /// d(mass fraction)/d(porous-flow variable)
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_frac_dvar;

  /// strain rate
  const MaterialProperty<Real> & _strain_rate_qp;

  /// d(strain rate)/d(porous-flow variable)
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

#endif // POROUSFLOWMASSVOLUMETRICEXPANSION_H
