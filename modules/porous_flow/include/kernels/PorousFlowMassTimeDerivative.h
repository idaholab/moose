/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMASSTIMEDERIVATIVE_H
#define PORFLOWMASSTIMEDERIVATIVE_H

#include "TimeDerivative.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowMassTimeDerivative;

template<>
InputParameters validParams<PorousFlowMassTimeDerivative>();

/**
 * Kernel = (mass_component - mass_component_old)/dt
 * where mass_component = porosity*sum_phases(density_phase*saturation_phase*massfrac_phase^component)
 * It is lumped to the nodes
 */
class PorousFlowMassTimeDerivative : public TimeKernel
{
public:

  PorousFlowMassTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _component_index;

  /// holds info on the PorousFlow variables
  const PorousFlowDictator & _dictator_UO;

  bool _var_is_porflow_var;

  const MaterialProperty<Real> & _porosity;

  const MaterialProperty<Real> & _porosity_old;

  const MaterialProperty<std::vector<Real> > & _dporosity_dvar;

  const MaterialProperty<std::vector<RealGradient> > & _dporosity_dgradvar;

  const MaterialProperty<std::vector<Real> > & _fluid_density;

  const MaterialProperty<std::vector<Real> > & _fluid_density_old;

  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_density_dvar;

  const MaterialProperty<std::vector<Real> > & _fluid_saturation_nodal;

  const MaterialProperty<std::vector<Real> > & _fluid_saturation_nodal_old;

  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_saturation_nodal_dvar;

  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac;

  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac_old;

  const MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _dmass_frac_dvar;

  /**
   * Derivative of residual with respect to PorousFlow variable number pvar
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param wrt_num take the derivative of the residual wrt this PorousFlow variable
   */
  Real computeQpJac(unsigned int pvar);

};

#endif //PORFLOWMASSTIMEDERIVATIVE_H
