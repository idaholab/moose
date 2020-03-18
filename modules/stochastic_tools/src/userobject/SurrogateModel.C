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

InputParameters
SurrogateModel::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += SamplerInterface::validParams();

  params.addRequiredParam<SamplerName>("training_sampler",
                                       "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");

  params.registerBase("SurrogateModel");
  return params;
}

SurrogateModel::SurrogateModel(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    SamplerInterface(this),
    _sampler(nullptr),
    _values(getVectorPostprocessorValue("results_vpp", getParam<std::string>("results_vector")))
{
}

void
SurrogateModel::initialSetup()
{
  _sampler = &getSamplerByName(getParam<SamplerName>("training_sampler"));

  _ndim = _sampler->getNumberOfCols();
}
