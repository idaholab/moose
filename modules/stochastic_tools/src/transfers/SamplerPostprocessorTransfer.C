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
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the Postprocessors on the sub-app to transfer from/to.");
  params.addRequiredParam<VectorPostprocessorName>("vector_postprocessor",
                                                   "The name of the VectorPostprocessor in "
                                                   "the MultiApp to transfer values "
                                                   "from/to.");
  params.set<MultiMooseEnum>("direction") = "from_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");
  return params;
}

SamplerPostprocessorTransfer::SamplerPostprocessorTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _sub_pp_name(getParam<PostprocessorName>("postprocessor")),
    _master_vpp_name(getParam<VectorPostprocessorName>("vector_postprocessor"))
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
  VectorPostprocessorValue & vpp =
      _fe_problem.getVectorPostprocessorValue(_master_vpp_name, _sampler_ptr->name(), false);

  if (_results->containsCompleteHistory())
    vpp.reserve(vpp.size() + _sampler_ptr->getNumberOfLocalRows());
  else
  {
    vpp.clear();
    vpp.reserve(_sampler_ptr->getNumberOfLocalRows());
  }
}

void
SamplerPostprocessorTransfer::executeFromMultiapp()
{
  VectorPostprocessorValue & vpp =
      _fe_problem.getVectorPostprocessorValue(_master_vpp_name, _sampler_ptr->name(), false);

  const dof_id_type n = _multi_app->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
  {
    if (_multi_app->hasLocalApp(i))
    {
      FEProblemBase & app_problem = _multi_app->appProblemBase(i);
      vpp.push_back(app_problem.getPostprocessorValue(_sub_pp_name));
    }
  }
}

void
SamplerPostprocessorTransfer::finalizeFromMultiapp()
{
}

void
SamplerPostprocessorTransfer::execute()
{
  VectorPostprocessorValue & vpp =
      _fe_problem.getVectorPostprocessorValue(_master_vpp_name, _sampler_ptr->name(), false);

  if (_results->containsCompleteHistory())
    vpp.reserve(vpp.size() + _sampler_ptr->getNumberOfLocalRows());
  else
  {
    vpp.clear();
    vpp.reserve(_sampler_ptr->getNumberOfLocalRows());
  }

  for (dof_id_type i = _sampler_ptr->getLocalRowBegin(); i < _sampler_ptr->getLocalRowEnd(); ++i)
  {
    FEProblemBase & app_problem = _multi_app->appProblemBase(i);
    vpp.emplace_back(app_problem.getPostprocessorValue(_sub_pp_name));
  }
}
