/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWCOMPONENTMASSTIMEDERIVATIVE_H
#define PORFLOWCOMPONENTMASSTIMEDERIVATIVE_H

#include "TimeDerivative.h"
#include "PorFlowVarNames.h"

// Forward Declarations
class PorFlowComponentMassTimeDerivative;

template<>
InputParameters validParams<PorFlowComponentMassTimeDerivative>();

/**
 * Kernel = (mass_component - mass_component_old)/dt
 * where mass_component = porosity*sum_phases(density_phase*saturation_phase*massfrac_phase^component)
 * It is lumped to the nodes
 */
class PorFlowComponentMassTimeDerivative : public TimeKernel
{
public:

  PorFlowComponentMassTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _component_index;

  /// holds info on the Richards variables
  const PorFlowVarNames & _porflow_name_UO;

  bool _var_is_porflow_var;

  const MaterialProperty<Real> & _porosity;

  const MaterialProperty<Real> & _porosity_old;

  const MaterialProperty<std::vector<Real> > & _dporosity_dvar;

  const MaterialProperty<std::vector<Real> > & _fluid_density;

  const MaterialProperty<std::vector<Real> > & _fluid_density_old;

  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_density_dvar;

  const MaterialProperty<std::vector<Real> > & _fluid_saturation;

  const MaterialProperty<std::vector<Real> > & _fluid_saturation_old;

  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_saturation_dvar;

  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac;

  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac_old;

  const MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _dmass_frac_dvar;

  /**
   * Derivative of residual with respect to PorFlow variable number pvar
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param wrt_num take the derivative of the residual wrt this PorFlow variable
   */
  Real computeQpJac(unsigned int pvar);

};

#endif //PORFLOWCOMPONENTMASSTIMEDERIVATIVE_H
