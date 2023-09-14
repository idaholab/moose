//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddOutputCovarianceAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "OutputCovarianceBase.h"

registerMooseAction("StochasticToolsApp", AddOutputCovarianceAction, "add_output_covariance");

InputParameters
AddOutputCovarianceAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds Output Covariance objects contained within "
                             "the `[Trainers]` and `[Surrogates]` input blocks.");
  return params;
}

AddOutputCovarianceAction::AddOutputCovarianceAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddOutputCovarianceAction::act()
{
  _problem->addObject<OutputCovarianceBase>(
      _type, _name, _moose_object_pars, /* threaded = */ false);
}
