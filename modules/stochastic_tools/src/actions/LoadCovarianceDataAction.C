//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LoadCovarianceDataAction.h"
#include "GaussianProcess.h"
#include "FEProblem.h"
#include "RestartableDataIO.h"
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
    auto * gp = dynamic_cast<GaussianProcess *>(model_ptr);
    if (gp && model_ptr->isParamValid("filename"))
      load(*gp);
  }
}

void
LoadCovarianceDataAction::load(GaussianProcess & model)
{
  const std::string & covar_type = model.getGPHandler().getCovarType();
  const std::unordered_map<std::string, Real> & map = model.getGPHandler().getHyperParamMap();
  const std::unordered_map<std::string, std::vector<Real>> & vec_map =
      model.getGPHandler().getHyperParamVectorMap();
  const UserObjectName & covar_name = model.name() + "_covar_func";

  InputParameters covar_params = _factory.getValidParams(covar_type);

  for (auto & p : map)
    covar_params.set<Real>(p.first) = p.second;

  for (auto & p : vec_map)
    covar_params.set<std::vector<Real>>(p.first) = p.second;

  _problem->addObject<CovarianceFunctionBase>(
      covar_type, covar_name, covar_params, /* threaded = */ false);

  model.setupCovariance(covar_name);
}
