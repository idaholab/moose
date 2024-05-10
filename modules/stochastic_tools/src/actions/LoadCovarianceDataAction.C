//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LoadCovarianceDataAction.h"
#include "GaussianProcessSurrogate.h"
#include "FEProblem.h"
#include "StochasticToolsApp.h"

registerMooseAction("StochasticToolsApp", LoadCovarianceDataAction, "load_covariance_data");

InputParameters
LoadCovarianceDataAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Calls load method on SurrogateModel objects contained within the "
                             "`[Surrogates]` input block, if a filename is given.");
  return params;
}

LoadCovarianceDataAction::LoadCovarianceDataAction(const InputParameters & params) : Action(params)
{
}

void
LoadCovarianceDataAction::act()
{
  std::vector<SurrogateModel *> objects;
  _app.theWarehouse().query().condition<AttribSystem>("SurrogateModel").queryInto(objects);
  for (auto model_ptr : objects)
  {
    auto * gp = dynamic_cast<GaussianProcessSurrogate *>(model_ptr);
    if (gp && model_ptr->isParamValid("filename"))
      load(*gp);
  }
}

void
LoadCovarianceDataAction::load(GaussianProcessSurrogate & model)
{
  const std::string & covar_type = model.getGPHandler().getCovarType();
  const std::string & covar_name = model.getGPHandler().getCovarName();
  const std::map<UserObjectName, std::string> & dep_covar_types =
      model.getGPHandler().getDependentCovarTypes();
  const unsigned int num_outputs = model.getGPHandler().getCovarNumOutputs();

  const std::unordered_map<std::string, Real> & map = model.getGPHandler().getHyperParamMap();
  const std::unordered_map<std::string, std::vector<Real>> & vec_map =
      model.getGPHandler().getHyperParamVectorMap();

  // We start by creating and loading the dependency function
  for (const auto & it : dep_covar_types)
  {
    const auto & name = it.first;
    const auto & type = it.second;
    InputParameters covar_params = _factory.getValidParams(type);

    const auto param_list = covar_params.getParametersList();
    for (const auto & param : param_list)
    {
      if (covar_params.isParamRequired(param))
      {
        const std::string expected_name = name + ":" + param;
        for (const auto & it_map : map)
        {
          const auto pos = it_map.first.find(expected_name);
          if (pos != std::string::npos)
            covar_params.set<Real>(param) = it_map.second;
        }
        for (const auto & it_map : vec_map)
        {
          const auto pos = it_map.first.find(expected_name);
          if (pos != std::string::npos)
            covar_params.set<std::vector<Real>>(param) = it_map.second;
        }
      };
    }

    _problem->addObject<CovarianceFunctionBase>(type, name, covar_params, /*threaded=*/false);
  }

  InputParameters covar_params = _factory.getValidParams(covar_type);

  const auto param_list = covar_params.getParametersList();
  for (const auto & param : param_list)
  {
    if (covar_params.isParamRequired(param))
    {
      const auto & map_it = map.find(param);
      if (map_it != map.end())
        covar_params.set<Real>(param) = map_it.second;

      const auto & vec_map_it = vec_map.find(param);
      if (vec_map_it != vec_map.end())
        covar_params.set<std::vector<Real>>(param) = vec_map_it.second;
    };
  }

  _problem->addObject<CovarianceFunctionBase>(
      covar_type, covar_name, covar_params, /* threaded = */ false);

  model.setupCovariance(covar_name);
}
