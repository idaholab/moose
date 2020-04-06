//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurrogateTrainer.h"
#include "Sampler.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

InputParameters
SurrogateTrainer::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += SamplerInterface::validParams();
  params.registerBase("SurrogateTrainer");
  return params;
}

SurrogateTrainer::SurrogateTrainer(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    SamplerInterface(this),
    _model_meta_data_name(_type + "_" + name())
{
  _app.registerRestartableDataMapName(_model_meta_data_name, name());
}
