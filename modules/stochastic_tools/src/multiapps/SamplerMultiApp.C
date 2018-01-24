//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SamplerMultiApp.h"

template <>
InputParameters
validParams<SamplerMultiApp>()
{
  InputParameters params = validParams<TransientMultiApp>();
  params.addClassDescription("Creates a sub-application for each row of each Sampler matrix.");
  params.addParam<SamplerName>("sampler", "The Sampler object to utilize for creating MultiApps.");
  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<Point>>("move_positions");
  params.suppressParameter<std::vector<unsigned int>>("move_apps");
  params.set<bool>("use_positions") = false;
  return params;
}

SamplerMultiApp::SamplerMultiApp(const InputParameters & parameters)
  : TransientMultiApp(parameters),
    SamplerInterface(this),
    _sampler(SamplerInterface::getSampler("sampler"))
{
  init(_sampler.getTotalNumberOfRows());
}
