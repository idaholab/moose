//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
