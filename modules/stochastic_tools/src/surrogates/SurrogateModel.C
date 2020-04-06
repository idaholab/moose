//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurrogateModel.h"
#include "SurrogateTrainer.h"
#include "Sampler.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

InputParameters
SurrogateModel::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SamplerInterface::validParams();
  params += SurrogateModelInterface::validParams();
  params.addParam<FileName>("filename", "Filename containing the trained data.");
  params.addParam<UserObjectName>(
      "trainer",
      "The SurrogateTrainer object. If this is specified the trainer data is automatically "
      "gathered and available in this SurrogateModel object.");
  params.registerBase("SurrogateModel");
  params.registerSystemAttributeName("SurrogateModel");
  return params;
}

SurrogateModel::SurrogateModel(const InputParameters & parameters)
  : MooseObject(parameters),
    SamplerInterface(this),
    SurrogateModelInterface(this),
    _model_meta_data_name(isParamValid("trainer")
                              ? getSurrogateTrainer("trainer").modelMetaDataName()
                              : _type + "_" + name())
{
  // Register the meta data that is going to be loaded into
  _app.registerRestartableDataMapName(_model_meta_data_name, name());
}
