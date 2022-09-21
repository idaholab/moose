//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddCovarianceAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "CovarianceFunctionBase.h"

registerMooseAction("StochasticToolsApp", AddCovarianceAction, "add_covariance");

InputParameters
AddCovarianceAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds Covariance objects contained within "
                             "the `[Trainers]` and `[Surrogates]` input blocks.");
  return params;
}

AddCovarianceAction::AddCovarianceAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddCovarianceAction::act()
{
  _problem->addObject<CovarianceFunctionBase>(
      _type, _name, _moose_object_pars, /* threaded = */ false);
}
