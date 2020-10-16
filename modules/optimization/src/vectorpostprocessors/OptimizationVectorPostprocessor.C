//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationVectorPostprocessor.h"

registerMooseObject("isopodApp", OptimizationVectorPostprocessor);

InputParameters
OptimizationVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Prints parameter results from optimization routine.");

  params.addRequiredParam<std::vector<std::string>>(
      "parameters",
      "A list of parameters (on the sub application) to control "
      "with the SamplerReceiver.");
  params.addRequiredParam<std::vector<Real>>(
      "intial_values", "A list of initial guesses for each controllable parameter.");

  return params;
}

OptimizationVectorPostprocessor::OptimizationVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _control_names(getParam<std::vector<std::string>>("parameters"))
{
  std::vector<Real> intial_values(getParam<std::vector<Real>>("intial_values"));
  if (_control_names.size() != intial_values.size())
    mooseError("The number parameters needs to match the number values");

  _vpp_vectors.resize(_control_names.size());
  for (std::size_t i = 0; i < _control_names.size(); ++i)
  {
    _vpp_vectors[i] = &declareVector(_control_names[i]);
    (*_vpp_vectors[i]).push_back(intial_values[i]);
  }
}

std::vector<std::string>
OptimizationVectorPostprocessor::getParameterNames()
{
  return _control_names;
}

std::vector<Real>
OptimizationVectorPostprocessor::getParameterValues()
{
  std::vector<Real> dataVec(_vpp_vectors.size());
  for (std::size_t i = 0; i < _vpp_vectors.size(); ++i)
  {
    dataVec[i] = (*_vpp_vectors[i])[0];
  }
  return dataVec;
}

void
OptimizationVectorPostprocessor::setParameterValues(const std::vector<Real> & current)
{
  mooseAssert(current.size() == _vpp_vectors.size(),
              "Receiving more values than declared columns in vpp");
  for (std::size_t i = 0; i < _vpp_vectors.size(); ++i)
  {
    mooseAssert((*_vpp_vectors[i]).size() == 1,
                "Each vector in OptimizationVectorPostprocessor vpp contains a single entry");
    (*_vpp_vectors[i])[0] = current[i];
  }
}
