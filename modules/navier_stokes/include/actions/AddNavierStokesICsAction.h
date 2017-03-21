/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDNAVIERSTOKESICSACTION_H
#define ADDNAVIERSTOKESICSACTION_H

#include "NSAction.h"

class AddNavierStokesICsAction;

template <>
InputParameters validParams<AddNavierStokesICsAction>();

/**
 * This class allows us to have a section of the input file like the
 * following which automatically adds initial conditions for all the
 * required nonlinear and auxiliary variables.
 *
 * [NavierStokes]
 *   [./ICs]
 *     initial_pressure = 101325.
 *     initial_temperature = 300.
 *     initial_velocity = '173.594354746921 0 0'
 *     fluid_properties = ideal_gas
 *   [../]
 * []
 */
class AddNavierStokesICsAction : public NSAction
{
public:
  AddNavierStokesICsAction(InputParameters parameters);
  virtual ~AddNavierStokesICsAction();

  virtual void act();

protected:
  // Helper function that actually adds the ICs.
  void addICs(std::vector<std::string> & names);

  Real _initial_pressure;
  Real _initial_temperature;
  RealVectorValue _initial_velocity;
  UserObjectName _fp_name;
};

#endif
