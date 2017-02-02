/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDNAVIERSTOKESVARIABLESACTION_H
#define ADDNAVIERSTOKESVARIABLESACTION_H

#include "NSAction.h"

class AddNavierStokesVariablesAction;

template <>
InputParameters validParams<AddNavierStokesVariablesAction>();

/**
 * This class allows us to have a section of the input file like the
 * following which automatically adds all the required nonlinear
 * variables with the appropriate scaling.
 *
 * [NavierStokes]
 *   [./Variables]
 *     #         'rho rhou  rhov  rhoE'
 *     scaling = '1.  1.    1.    9.869232667160121e-6'
 *     family = LAGRANGE
 *     order = FIRST
 *   [../]
 * []
 */
class AddNavierStokesVariablesAction : public NSAction
{
public:
  AddNavierStokesVariablesAction(InputParameters parameters);
  virtual ~AddNavierStokesVariablesAction();

  virtual void act();

protected:
  std::vector<Real> _scaling;
  std::vector<SubdomainName> _blocks;
};

#endif /* ADDNAVIERSTOKESVARIABLESACTION_H */
