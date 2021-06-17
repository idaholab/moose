//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "StochasticReporter.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", StochasticReporter);

InputParameters
StochasticReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Storage container for stochastic simulation results coming from Reporters.");
  return params;
}

StochasticReporter::StochasticReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _converged(declareValueByName<std::vector<bool>, ReporterVectorContext<bool>>(
        convergedReporterName(), REPORTER_MODE_DISTRIBUTED))
{
}
