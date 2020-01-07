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

defineLegacyParams(SamplerPostprocessorTransfer);

InputParameters
SamplerPostprocessorTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Transfers data from Postprocessors on the sub-application to a "
                             "VectorPostprocessor on the master application.");
  params.addParam<PostprocessorName>(
      "from_postprocessor", "The name of the Postprocessors on the sub-app to transfer from.");
  params.addParam<VectorPostprocessorName>("to_vector_postprocessor",
                                           "The name of the VectorPostprocessor in "
                                           "the MultiApp to transfer values "
                                           "to.");

  params.addDeprecatedParam<PostprocessorName>(
      "postprocessor",
      "The name of the Postprocessors on the sub-app to transfer from.",
      "Replaced by from_postprocessor");
  params.addDeprecatedParam<VectorPostprocessorName>("vector_postprocessor",
                                                     "The name of the VectorPostprocessor in "
                                                     "the MultiApp to transfer values "
                                                     "to.",
                                                     "Replaced by to_vector_postprocessor");

  params.set<MultiMooseEnum>("direction") = "from_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");
  return params;
}

SamplerPostprocessorTransfer::SamplerPostprocessorTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _sub_pp_name(isParamValid("postprocessor") ? getParam<PostprocessorName>("postprocessor")
                                               : getParam<PostprocessorName>("from_postprocessor")),
    _master_vpp_name(isParamValid("vector_postprocessor")
                         ? getParam<VectorPostprocessorName>("vector_postprocessor")
                         : getParam<VectorPostprocessorName>("to_vector_postprocessor"))
{
}

void
SamplerPostprocessorTransfer::initialSetup()
{
  auto & uo = _fe_problem.getUserObjectTempl<UserObject>(_master_vpp_name);
  _results = dynamic_cast<StochasticResults *>(&uo);

  if (!_results)
    mooseError("The 'results' object must be a 'StochasticResults' object.");

  _results->init(*_sampler_ptr);
}

void
SamplerPostprocessorTransfer::initializeFromMultiapp()
{
  _current_data.clear();
  _current_data.reserve(_sampler_ptr->getNumberOfLocalRows());
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
      _current_data.emplace_back(app_problem.getPostprocessorValue(_sub_pp_name));
    }
  }
}

void
SamplerPostprocessorTransfer::finalizeFromMultiapp()
{
  _results->setCurrentLocalVectorPostprocessorValue(_sampler_ptr->name(), std::move(_current_data));
  _current_data.clear();
}

void
SamplerPostprocessorTransfer::execute()
{
  VectorPostprocessorValue current;
  current.reserve(_sampler_ptr->getNumberOfLocalRows());
  for (dof_id_type i = _sampler_ptr->getLocalRowBegin(); i < _sampler_ptr->getLocalRowEnd(); ++i)
  {
    FEProblemBase & app_problem = _multi_app->appProblemBase(i);
    current.emplace_back(app_problem.getPostprocessorValue(_sub_pp_name));
  }
  _results->setCurrentLocalVectorPostprocessorValue(_sampler_ptr->name(), std::move(current));
}
