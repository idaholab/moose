//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurrogateModel.h"
#include "Sampler.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

InputParameters
SurrogateModel::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += SamplerInterface::validParams();
  params.addParam<FileName>("filename", "Filename containing the trained data.");
  params.registerBase("SurrogateModel");
  return params;
}

SurrogateModel::SurrogateModel(const InputParameters & parameters)
  : GeneralUserObject(parameters), SamplerInterface(this), _is_training(!isParamValid("filename"))
{
  _app.registerRestartableDataMapName(name(), name());
  declareModelData("type", getParam<std::string>("_type"));
}

bool
SurrogateModel::isTraining() const
{
  return _is_training;
}

void
SurrogateModel::initialize()
{
  if (isTraining())
    trainInitialize();
}

void
SurrogateModel::execute()
{
  if (isTraining())
    train();
}

void
SurrogateModel::finalize()
{
  if (isTraining())
    trainFinalize();
}
