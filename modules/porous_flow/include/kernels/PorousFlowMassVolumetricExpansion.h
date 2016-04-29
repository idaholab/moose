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

template<>
InputParameters validParams<PorousFlowMassVolumetricExpansion>();

/**
 * Kernel = mass_component * d(volumetric_strain)/dt
 * where mass_component = porosity*sum_phases(density_phase*saturation_phase*massfrac_phase^component)
 * which is lumped to the nodes
 */
class PorousFlowMassVolumetricExpansion : public TimeKernel
{
public:
  PorousFlowMassVolumetricExpansion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// the fluid component index
  const unsigned int _component_index;

  /// holds info on the Richards variables
  const PorousFlowDictator & _dictator_UO;

  /// whether the Variable for this Kernel is a porous-flow variable according to the Dictator
  const bool _var_is_porflow_var;

  /// number of displacement variables
  unsigned int _ndisp;

  /// variable number of the displacements variables
  std::vector<unsigned int> _disp_var_num;

  const MaterialProperty<Real> & _porosity;

  const MaterialProperty<std::vector<Real> > & _dporosity_dvar;

  const MaterialProperty<std::vector<RealGradient> > & _dporosity_dgradvar;

  const MaterialProperty<std::vector<Real> > & _fluid_density;

  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_density_dvar;

  const MaterialProperty<std::vector<Real> > & _fluid_saturation;

  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_saturation_dvar;

  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac;

  const MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _dmass_frac_dvar;

  const MaterialProperty<Real> & _strain_rate_qp;

  const MaterialProperty<std::vector<RealGradient> > & _dstrain_rate_qp_dvar;

  /**
   * Derivative of mass part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the mass part of the residual wrt this variable number
   */
  Real computedMassQpJac(unsigned int jvar);

  /**
   * Derivative of volumetric-strain part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the volumetric-strain part of the residual wrt this variable number
   */
  Real computedVolQpJac(unsigned int pvar);
};

#endif //POROUSFLOWMASSVOLUMETRICEXPANSION_H
