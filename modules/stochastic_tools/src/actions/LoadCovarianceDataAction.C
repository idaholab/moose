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
    auto * gp_gen = dynamic_cast<GaussianProcessSurrogate *>(model_ptr);
    if (gp_gen && model_ptr->isParamValid("filename"))
      load(*gp_gen);
  }
}

void
LoadCovarianceDataAction::load(GaussianProcessSurrogate & model)
{
  // We grab all the necessary information that is needed to reconstruct the
  // covariance structure for the GP
  const std::string & covar_type = model.getGP().getCovarType();
  const std::string & covar_name = model.getGP().getCovarName();
  const std::map<UserObjectName, std::string> & dep_covar_types =
      model.getGP().getDependentCovarTypes();
  const std::vector<UserObjectName> & dep_covar_names = model.getGP().getDependentCovarNames();

  // This is for the covariance on the very top, the lower-level covariances are
  // all assumed to have num_outputs=1.
  const unsigned int num_outputs = model.getGP().getCovarNumOutputs();
  const std::unordered_map<std::string, Real> & map = model.getGP().getHyperParamMap();
  const std::unordered_map<std::string, std::vector<Real>> & vec_map =
      model.getGP().getHyperParamVectorMap();

  // We start by creating and loading the lower-level covariances if they need
  // to be present. Right now we can only load a complex covariance which has
  // a one-level dependency depth.
  // TODO: Extend this to arbitrary dependency depths. Maybe we could use a graph.
  for (const auto & it : dep_covar_types)
  {
    const auto & name = it.first;
    const auto & type = it.second;
    InputParameters covar_params = _factory.getValidParams(type);

    // We make sure that every required parameter is added so that the object
    // can be constructed. The non-required hyperparameters (if present in the
    // parameter maps) will be inserted later.
    const auto param_list = covar_params.getParametersList();
    for (const auto & param : param_list)
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
      }

    _problem->addObject<CovarianceFunctionBase>(type, name, covar_params, /*threaded=*/false);
  }

  InputParameters covar_params = _factory.getValidParams(covar_type);
  covar_params.set<unsigned int>("num_outputs") = num_outputs;
  covar_params.set<std::vector<UserObjectName>>("covariance_functions") = dep_covar_names;

  const auto param_list = covar_params.getParametersList();
  for (const auto & param : param_list)
    // We make sure that every required parameter is added so that the object
    // can be constructed. The non-required hyperparameters (if present in the
    // parameter maps) will be inserted later.
    if (covar_params.isParamRequired(param))
    {
      const std::string expected_name = covar_name + ":" + param;

      const auto & map_it = map.find(expected_name);
      if (map_it != map.end())
        covar_params.set<Real>(param) = map_it->second;

      const auto & vec_map_it = vec_map.find(expected_name);
      if (vec_map_it != vec_map.end())
        covar_params.set<std::vector<Real>>(param) = vec_map_it->second;
    }

  auto covar_object = _problem->addObject<CovarianceFunctionBase>(
      covar_type, covar_name, covar_params, /* threaded = */ false);
  covar_object[0]->loadHyperParamMap(map, vec_map);

  model.setupCovariance(covar_name);
}
