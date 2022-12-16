//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurrogateModelScalarAux.h"

registerMooseObject("StochasticToolsApp", SurrogateModelScalarAux);

InputParameters
SurrogateModelScalarAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription("Sets a value of a scalar variable based on a surrogate model.");
  params.addRequiredParam<UserObjectName>("model", "Name of surrogate models.");
  params.addRequiredParam<std::vector<std::string>>(
      "parameters",
      "Parameter values at which the surrogate is evaluated. Accepts scalar variables, "
      "postprocessors, and real numbers.");
  return params;
}

SurrogateModelScalarAux::SurrogateModelScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters), SurrogateModelInterface(this), _model(getSurrogateModel("model"))
{
}

void
SurrogateModelScalarAux::initialSetup()
{
  const auto parameter_names = getParam<std::vector<std::string>>("parameters");
  _n_params = parameter_names.size();
  for (unsigned int j = 0; j < _n_params; ++j)
  {
    const auto name = parameter_names[j];

    // name can be a postprocessor, scalar var, or real number
    if (_sc_fe_problem.hasScalarVariable(name))
    {
      auto & scalar_val = _sc_fe_problem.getScalarVariable(_tid, name).sln()[0];
      _scalar_var_params.push_back(&scalar_val);
      _scalar_var_indices.push_back(j);
    }
    else if (_sc_fe_problem.hasPostprocessorValueByName(name))
    {
      _pp_indices.push_back(j);
      _pp_params.push_back(&getPostprocessorValueByName(name));
    }
    else
    {
      // first check if the entry is a real
      std::stringstream ss(name);
      Real v;
      ss >> v;

      if (ss.fail())
        paramError("parameters",
                   "Parameter ",
                   name,
                   " is not a scalar variable, postprocessor, or real number");

      _real_params.push_back(v);
      _real_indices.push_back(j);
    }
  }
  _n_scalar = _scalar_var_indices.size();
  _n_pp = _pp_indices.size();
  _n_real = _real_indices.size();
}

Real
SurrogateModelScalarAux::computeValue()
{
  std::vector<Real> x(_n_params);
  // insert scalar variables
  for (unsigned int j = 0; j < _n_scalar; ++j)
    x[_scalar_var_indices[j]] = *_scalar_var_params[j];
  // insert postprocessors
  for (unsigned int j = 0; j < _n_pp; ++j)
    x[_pp_indices[j]] = *_pp_params[j];
  // insert real values
  for (unsigned int j = 0; j < _n_real; ++j)
    x[_real_indices[j]] = _real_params[j];
  return _model.evaluate(x);
}
