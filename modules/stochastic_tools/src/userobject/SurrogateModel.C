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

defineLegacyParams(SurrogateModel);

InputParameters
SurrogateModel::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += SamplerInterface::validParams();

  params.addRequiredParam<SamplerName>("training_sampler",
                                       "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "stochastic_results", "Vectorpostprocessor with results of samples created by trainer");

  params.registerBase("SurrogateModel");
  return params;
}

SurrogateModel::SurrogateModel(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    SamplerInterface(this),
    _sampler(nullptr),
    _values(getVectorPostprocessorValue("stochastic_results",
                                        getParam<SamplerName>("training_sampler")))
{
}

void
SurrogateModel::initialSetup()
{
  _sampler = &getSamplerByName(getParam<SamplerName>("training_sampler"));

  _ndim = _sampler->getNumberOfCols();
}
