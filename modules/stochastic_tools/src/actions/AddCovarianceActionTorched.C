//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddCovarianceActionTorched.h"
#include "Factory.h"
#include "FEProblem.h"
#include "CovarianceFunctionBaseTorched.h"

registerMooseAction("StochasticToolsApp", AddCovarianceActionTorched, "add_covariance_torched");

InputParameters
AddCovarianceActionTorched::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds Covariance objects contained within "
                             "the `[Trainers]` and `[Surrogates]` input blocks.");
  return params;
}

AddCovarianceActionTorched::AddCovarianceActionTorched(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddCovarianceActionTorched::act()
{
  _problem->addObject<CovarianceFunctionBaseTorched>(
      _type, _name, _moose_object_pars, /* threaded = */ false);
}
