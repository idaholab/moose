/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef Q2PACTION_H
#define Q2PACTION_H

#include "Action.h"

class Q2PAction;

template <>
InputParameters validParams<Q2PAction>();

class Q2PAction : public Action
{
public:
  Q2PAction(const InputParameters & params);

  virtual void act();

private:
  VariableName _pp_var;
  VariableName _sat_var;
  UserObjectName _water_density;
  UserObjectName _water_relperm;
  UserObjectName _water_relperm_for_diffusivity;
  Real _water_viscosity;
  UserObjectName _gas_density;
  UserObjectName _gas_relperm;
  Real _gas_viscosity;
  Real _diffusivity;
  std::vector<OutputName> _output_nodal_masses_to;
  std::vector<OutputName> _output_total_masses_to;
  bool _save_gas_flux_in_Q2PGasFluxResidual;
  bool _save_water_flux_in_Q2PWaterFluxResidual;
  bool _save_gas_Jacobian_in_Q2PGasJacobian;
  bool _save_water_Jacobian_in_Q2PWaterJacobian;
  bool _nodal_masses_not_outputted;
  bool _total_masses_not_outputted;
  bool _no_mass_calculations;
};

#endif // Q2PACTION_H
