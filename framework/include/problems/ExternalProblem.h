//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXTERNALPROBLEM_H
#define EXTERNALPROBLEM_H

#include "FEProblemBase.h"

class ExternalProblem;

template <>
InputParameters validParams<ExternalProblem>();

class ExternalProblem : public FEProblemBase
{
public:
  ExternalProblem(const InputParameters & parameters);

  virtual void solve() override = 0;
  virtual bool converged() override = 0;

  /**
   * Method called to add AuxVariables to the simulation. These variables would be the fields that
   * should either be saved out with the MOOSE-formatted solutions or available for transfer to
   * variables in Multiapp simulations.
   */
  virtual void addExternalVariables() {}
};

#endif /* EXTERNALPROBLEM_H */
