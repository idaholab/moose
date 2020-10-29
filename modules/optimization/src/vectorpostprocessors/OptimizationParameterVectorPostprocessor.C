//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationParameterVectorPostprocessor.h"

registerMooseObject("isopodApp", OptimizationParameterVectorPostprocessor);

InputParameters
OptimizationParameterVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Prints parameter results from optimization routine.");

  params.addRequiredParam<std::vector<std::string>>(
      "parameters",
      "A list of parameters (on the sub application) to control "
      "with the SamplerReceiver.");

  return params;
}

OptimizationParameterVectorPostprocessor::OptimizationParameterVectorPostprocessor(
    const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _control_names(getParam<std::vector<std::string>>("parameters"))
{

  _vpp_vectors.resize(_control_names.size());
  for (std::size_t i = 0; i < _control_names.size(); ++i)
  {
    _vpp_vectors[i] = &declareVector(_control_names[i]);
    (*_vpp_vectors[i]).push_back(0);
  }
}

std::vector<std::string>
OptimizationParameterVectorPostprocessor::getParameterNames()
{
  return _control_names;
}

std::vector<Real>
OptimizationParameterVectorPostprocessor::getParameterValues()
{
  std::vector<Real> dataVec(_vpp_vectors.size());
  for (std::size_t i = 0; i < _vpp_vectors.size(); ++i)
  {
    dataVec[i] = (*_vpp_vectors[i])[0];
  }
  return dataVec;
}

dof_id_type
OptimizationParameterVectorPostprocessor::getNumberOfParameters()
{
  return _vpp_vectors.size();
}

void
OptimizationParameterVectorPostprocessor::setParameterValues(const std::vector<Real> & current)
{
  mooseAssert(current.size() == _vpp_vectors.size(),
              "Receiving more values than declared columns in vpp");
  for (std::size_t i = 0; i < _vpp_vectors.size(); ++i)
  {
    mooseAssert(
        (*_vpp_vectors[i]).size() == 1,
        "Each vector in OptimizationParameterVectorPostprocessor vpp contains a single entry");
    (*_vpp_vectors[i])[0] = current[i];
  }
}
