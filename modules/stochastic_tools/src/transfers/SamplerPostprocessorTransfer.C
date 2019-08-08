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

registerMooseObject("StochasticToolsApp", SamplerPostprocessorTransfer);

template <>
InputParameters
validParams<SamplerPostprocessorTransfer>()
{
  InputParameters params = validParams<StochasticToolsTransfer>();
  params.addClassDescription("Transfers data from Postprocessors on the sub-application to a "
                             "VectorPostprocessor on the master application.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the Postprocessors on the sub-app to transfer from/to.");
  params.addRequiredParam<VectorPostprocessorName>("vector_postprocessor",
                                                   "The name of the VectorPostprocessor in "
                                                   "the MultiApp to transfer values "
                                                   "from/to.");
  params.set<MooseEnum>("direction") = "from_multiapp";
  params.suppressParameter<MooseEnum>("direction");
  return params;
}

SamplerPostprocessorTransfer::SamplerPostprocessorTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _sub_pp_name(getParam<PostprocessorName>("postprocessor")),
    _master_vpp_name(getParam<VectorPostprocessorName>("vector_postprocessor"))
{
  // Determine the Sampler
  std::shared_ptr<SamplerTransientMultiApp> ptr_transient =
      std::dynamic_pointer_cast<SamplerTransientMultiApp>(_multi_app);
  std::shared_ptr<SamplerFullSolveMultiApp> ptr_fullsolve =
      std::dynamic_pointer_cast<SamplerFullSolveMultiApp>(_multi_app);

  if (!ptr_transient && !ptr_fullsolve)
    mooseError("The 'multi_app' parameter must provide either a 'SamplerTransientMultiApp' or "
               "'SamplerFullSolveMultiApp' object.");

  if (ptr_transient)
    _sampler = &(ptr_transient->getSampler());
  else
    _sampler = &(ptr_fullsolve->getSampler());
}

void
SamplerPostprocessorTransfer::initialSetup()
{
  auto & uo = _fe_problem.getUserObjectTempl<UserObject>(_master_vpp_name);
  _results = dynamic_cast<StochasticResults *>(&uo);

  if (!_results)
    mooseError("The 'results' object must be a 'StochasticResults' object.");

  _results->init(*_sampler);
}

void
SamplerPostprocessorTransfer::initializeFromMultiapp()
{
  _local_values.clear();
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
      _local_values.push_back(app_problem.getPostprocessorValue(_sub_pp_name));
    }
  }
}

void
SamplerPostprocessorTransfer::finalizeFromMultiapp()
{
  // Gather the PP values from all ranks
  _communicator.gather(0, _local_values);

  // Update VPP
  if (processor_id() == 0)
  {
    _results->initialize();
    const dof_id_type n = (_sampler->getNumberOfMatrices()) * (_sampler->getNumberOfSamples());
    for (MooseIndex(n) i = 0; i < n; i++)
    {
      Sampler::Location loc = _sampler->getLocation(i);
      VectorPostprocessorValue & vpp = _results->getVectorPostprocessorValueByGroup(loc.sample());
      vpp[loc.row()] = _local_values[i];
    }
  }
}

void
SamplerPostprocessorTransfer::execute()
{
  // Number of PP is equal to the number of MultiApps
  const unsigned int n = _multi_app->numGlobalApps();

  // Collect the PP values for this processor
  _local_values.assign(n, 0);
  for (unsigned int i = 0; i < n; i++)
  {
    if (_multi_app->hasLocalApp(i))
    {
      FEProblemBase & app_problem = _multi_app->appProblemBase(i);

      // use reserve and push_back b/c access to FEProblemBase is based on global id
      _local_values[i] = app_problem.getPostprocessorValue(_sub_pp_name);
    }
  }

  // Sum the PP values from all ranks
  _communicator.sum(_local_values);

  // Initialize VPP
  _results->initialize();

  // Update VPP
  for (unsigned int i = 0; i < n; i++)
  {
    Sampler::Location loc = _sampler->getLocation(i);
    VectorPostprocessorValue & vpp = _results->getVectorPostprocessorValueByGroup(loc.sample());
    vpp[loc.row()] = _local_values[i];
  }
}
