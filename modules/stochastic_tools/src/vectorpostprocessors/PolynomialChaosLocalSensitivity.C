//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "PolynomialChaosLocalSensitivity.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsApp", PolynomialChaosLocalSensitivity);

InputParameters
PolynomialChaosLocalSensitivity::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription(
      "Tool for calculating local sensitivity with polynomial chaos expansion.");
  params += SamplerInterface::validParams();

  params.addRequiredParam<UserObjectName>("pc_name", "Name of PolynomialChaos.");

  params.addParam<std::vector<unsigned int>>(
      "sensitivity_parameters",
      std::vector<unsigned int>(0),
      "Parameters to take derivative with respect to, matchs columns in training sampler. "
      "Default is to find local sensitivity of all parameters.");
  params.addParam<std::vector<Real>>(
      "local_points",
      std::vector<Real>(0),
      "Points specifying desired location of sensitivity measurement.");
  params.addParam<SamplerName>("local_points_sampler",
                               "Sampler specifying desired location of sensitivity measurement.");
  params.addParam<bool>("output_points", false, "True to include local points in output.");
  return params;
}

PolynomialChaosLocalSensitivity::PolynomialChaosLocalSensitivity(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerInterface(this),
    SurrogateModelInterface(this),
    _pc_uo(getSurrogateModel<PolynomialChaos>("pc_name")),
    _sampler(isParamValid("local_points_sampler") ? &getSampler("local_points_sampler") : nullptr),
    _points(getParam<std::vector<Real>>("local_points")),
    _sdim(getParam<std::vector<unsigned int>>("sensitivity_parameters")),
    _output_points(getParam<bool>("output_points")),
    _initialized(false)
{
  if (!_sampler && _points.size() == 0)
    paramError(
        "local_points",
        "Must specify either local_points or local_points_sampler to obtain local sensitivities.");
}

void
PolynomialChaosLocalSensitivity::initialize()
{
  // This needs to be put in initialize since the user object doesn't know the
  // number of parameters until after initialSetup.
  if (!_initialized)
  {
    if (_sdim.size() == 0)
    {
      _sdim.resize(_pc_uo.getNumberOfParameters());
      for (unsigned int d = 0; d < _pc_uo.getNumberOfParameters(); ++d)
        _sdim[d] = d;
    }

    if (_points.size() % _pc_uo.getNumberOfParameters() != 0)
      mooseError("Number of values in local_points is not divisible by number of parameters in "
                 "Polynomial Chaos model.");
    else if (_sampler)
    {
      if (_sampler->getNumberOfCols() != _pc_uo.getNumberOfParameters())
        mooseError("Number of columns in local_points_sampler is not equal to the number of "
                   "parameters in Polynomial Chaos model.");
    }

    if (_output_points)
    {
      _points_vector.reserve(_pc_uo.getNumberOfParameters());
      for (unsigned int d = 0; d < _pc_uo.getNumberOfParameters(); ++d)
        _points_vector.push_back(&declareVector("local_point_" + std::to_string(d)));
    }

    _sensitivity_vector.reserve(_sdim.size());
    for (auto d : _sdim)
    {
      if (d >= _pc_uo.getNumberOfParameters())
        mooseError("Specified sensitivity_parameter exceeds number of parameters in Polynomial "
                   "Chaos model.");
      _sensitivity_vector.push_back(&declareVector("sensitivity_" + std::to_string(d)));
    }
  }

  _initialized = true;

  unsigned int npoints = (_sampler ? _sampler->getNumberOfRows() : 0) +
                         (_points.size() / _pc_uo.getNumberOfParameters());

  if (_output_points)
  {
    for (unsigned int d = 0; d < _pc_uo.getNumberOfParameters(); ++d)
      _points_vector[d]->resize(npoints, 0);
  }

  for (unsigned int i = 0; i < _sdim.size(); ++i)
    _sensitivity_vector[i]->resize(npoints, 0);
}

void
PolynomialChaosLocalSensitivity::execute()
{
  // Loop over samples
  if (_sampler)
  {
    for (dof_id_type p = _sampler->getLocalRowBegin(); p < _sampler->getLocalRowEnd(); ++p)
    {
      std::vector<Real> data = _sampler->getNextLocalRow();
      if (_output_points)
        for (unsigned int d = 0; d < _pc_uo.getNumberOfParameters(); ++d)
          (*_points_vector[d])[p] = data[d];
      Real val = _pc_uo.evaluate(data);
      for (unsigned int i = 0; i < _sdim.size(); ++i)
        (*_sensitivity_vector[i])[p] =
            data[_sdim[i]] / val * _pc_uo.computeDerivative(_sdim[i], data);
    }
  }

  // Loop over specific points
  if (_points.size() > 0)
  {
    dof_id_type n_local = _points.size() / _pc_uo.getNumberOfParameters();
    dof_id_type st_local = 0;
    dof_id_type end_local = n_local;
    MooseUtils::linearPartitionItems(
        n_local, n_processors(), processor_id(), n_local, st_local, end_local);

    for (dof_id_type p = st_local; p < end_local; ++p)
    {
      dof_id_type vp = (_sampler ? _sampler->getNumberOfRows() : 0) + p;
      std::vector<Real> data(_pc_uo.getNumberOfParameters());
      for (unsigned int d = 0; d < _pc_uo.getNumberOfParameters(); ++d)
      {
        data[d] = _points[p * _pc_uo.getNumberOfParameters() + d];
        if (_output_points)
          (*_points_vector[d])[vp] = data[d];
      }
      Real val = _pc_uo.evaluate(data);
      for (unsigned int i = 0; i < _sdim.size(); ++i)
        (*_sensitivity_vector[i])[vp] =
            data[_sdim[i]] / val * _pc_uo.computeDerivative(_sdim[i], data);
    }
  }
}

void
PolynomialChaosLocalSensitivity::finalize()
{
  if (_output_points)
    for (auto & ppv_ptr : _points_vector)
      gatherSum(*ppv_ptr);
  for (auto & ppv_ptr : _sensitivity_vector)
    gatherSum(*ppv_ptr);
}
