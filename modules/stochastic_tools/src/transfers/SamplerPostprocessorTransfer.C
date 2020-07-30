//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SamplerPostprocessorTransfer.h"
#include "SamplerFullSolveMultiApp.h"
#include "SamplerTransientMultiApp.h"
#include "SamplerReceiver.h"
#include "StochasticResults.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", SamplerPostprocessorTransfer);

std::vector<VectorPostprocessorName>
getVectorNamesHelper(const std::string & prefix, const std::vector<PostprocessorName> & pp_names)
{
  std::vector<VectorPostprocessorName> vec_names;
  vec_names.reserve(pp_names.size());
  for (const auto & pp_name : pp_names)
  {
    if (!prefix.empty())
      vec_names.push_back(prefix + ":" + pp_name);
    else
      vec_names.push_back(pp_name);
  }
  return vec_names;
}

InputParameters
SamplerPostprocessorTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Transfers data from Postprocessors on the sub-application to a "
                             "VectorPostprocessor on the master application.");

  params.addParam<std::vector<PostprocessorName>>(
      "from_postprocessor", "The name(s) of the Postprocessor(s) on the sub-app to transfer from.");
  params.addParam<VectorPostprocessorName>("to_vector_postprocessor",
                                           "The name of the VectorPostprocessor in "
                                           "the MultiApp to transfer values "
                                           "to.");

  params.addParam<std::string>("prefix",
                               "Use the supplied string as the prefix for vector postprocessor "
                               "name rather than the transfer name.");

  params.addParam<bool>("skip_solve_not_converge_value",
                        false,
                        "True to skip entries where the sub app did not converge.");
  params.addParam<Real>("solve_not_converge_value",
                        std::numeric_limits<double>::quiet_NaN(),
                        "Value to transfer if the sub app solve did not converge. If not "
                        "specified, whatever the value the sub app has upon exitting is used.");

  params.set<MultiMooseEnum>("direction") = "from_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");
  return params;
}

SamplerPostprocessorTransfer::SamplerPostprocessorTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _sub_pp_names(getParam<std::vector<PostprocessorName>>("from_postprocessor")),
    _master_vpp_name(getParam<VectorPostprocessorName>("to_vector_postprocessor")),
    _vpp_names(isParamValid("prefix")
                   ? getVectorNamesHelper(getParam<std::string>("prefix"), _sub_pp_names)
                   : getVectorNamesHelper(_name, _sub_pp_names)),
    _skip_diverge(getParam<bool>("skip_solve_not_converge_value")),
    _diverge_value(getParam<Real>("solve_not_converge_value"))
{
}

const std::vector<VectorPostprocessorName> &
SamplerPostprocessorTransfer::vectorNames() const
{
  return _vpp_names;
}

void
SamplerPostprocessorTransfer::initialSetup()
{
  // Get the StochasticResults VPP object to populate
  auto & uo = _fe_problem.getUserObject<UserObject>(_master_vpp_name);
  _results = dynamic_cast<StochasticResults *>(&uo);
  if (!_results)
    mooseError("The 'results' object must be a 'StochasticResults' object.");

  // Check that postprocessor on sub-application exists and create vectors on results VPP
  const dof_id_type n = _multi_app->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
  {
    if (_multi_app->hasLocalApp(i))
    {
      FEProblemBase & app_problem = _multi_app->appProblemBase(i);
      for (const auto & sub_pp_name : _sub_pp_names)
        if (!app_problem.hasPostprocessor(sub_pp_name))
          mooseError("Unknown postprocesssor name '",
                     sub_pp_name,
                     "' on sub-application '",
                     _multi_app->name(),
                     "'");
    }
  }

  // Initialize storage for accumulating VPP data
  _current_data.resize(_sub_pp_names.size());
}

void
SamplerPostprocessorTransfer::initializeFromMultiapp()
{
  for (VectorPostprocessorValue & current : _current_data)
  {
    current.clear();
    current.reserve(_sampler_ptr->getNumberOfLocalRows());
  }
}

void
SamplerPostprocessorTransfer::executeFromMultiapp()
{
  const dof_id_type n = _multi_app->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
  {
    if (_multi_app->hasLocalApp(i))
    {
      FEProblemBase & app_problem = _multi_app->appProblemBase(i);
      if (app_problem.converged() || !_skip_diverge)
      {
        if (app_problem.converged() || std::isnan(_diverge_value))
          for (std::size_t j = 0; j < _sub_pp_names.size(); ++j)
            _current_data[j].emplace_back(app_problem.getPostprocessorValue(_sub_pp_names[j]));
        else
          for (std::size_t j = 0; j < _sub_pp_names.size(); ++j)
            _current_data[j].emplace_back(_diverge_value);
      }
    }
  }
}

void
SamplerPostprocessorTransfer::finalizeFromMultiapp()
{
  for (std::size_t j = 0; j < _sub_pp_names.size(); ++j)
  {
    _results->setCurrentLocalVectorPostprocessorValue(_vpp_names[j], std::move(_current_data[j]));
    _current_data[j].clear();
  }
}

void
SamplerPostprocessorTransfer::execute()
{
  for (std::size_t j = 0; j < _sub_pp_names.size(); ++j)
  {
    VectorPostprocessorValue current;
    current.reserve(_sampler_ptr->getNumberOfLocalRows());
    for (dof_id_type i = _sampler_ptr->getLocalRowBegin(); i < _sampler_ptr->getLocalRowEnd(); ++i)
    {
      FEProblemBase & app_problem = _multi_app->appProblemBase(i);
      if (app_problem.converged() || !_skip_diverge)
      {
        if (app_problem.converged() || std::isnan(_diverge_value))
          current.emplace_back(app_problem.getPostprocessorValue(_sub_pp_names[j]));
        else
          current.emplace_back(_diverge_value);
      }
    }
    _results->setCurrentLocalVectorPostprocessorValue(_vpp_names[j], std::move(current));
  }
}
