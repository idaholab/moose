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
#include "StochasticToolsTypes.h"

#include <unistd.h>

registerMooseObject("StochasticToolsApp", SamplerPostprocessorTransfer);

template <>
InputParameters
validParams<SamplerPostprocessorTransfer>()
{
  InputParameters params = validParams<MultiAppVectorPostprocessorTransfer>();
  params.addClassDescription("Transfers data from Postprocessors on the sub-application.");
  params.set<MooseEnum>("direction") = "from_multiapp";
  params.set<std::string>("vector_name") = "";
  params.suppressParameter<MooseEnum>("direction");
  params.suppressParameter<std::string>("vector_name");

  params.set<ExecFlagEnum>("execute_on", true).addAvailableFlags(StochasticTools::EXEC_PRE_BATCH_MULTIAPP,
                                                                 StochasticTools::EXEC_BATCH_MULTIAPP,
                                                                 StochasticTools::EXEC_POST_BATCH_MULTIAPP);
  return params;
}

SamplerPostprocessorTransfer::SamplerPostprocessorTransfer(const InputParameters & parameters)
  : MultiAppVectorPostprocessorTransfer(parameters)
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
  {
    _sampler = &(ptr_fullsolve->getSampler());
    _batch_mode = ptr_fullsolve->getParam<MooseEnum>("mode") == "batch";
  }

  // When the MultiApp object is running with 'mode=batch' the normal execution flags are not
  // adequate. This is because the MultiApp will run one solve, then reset and run another solve.
  // Thus, to get data from this operation mode the Transfer objects are required to execute
  // along with these run/reset executions. To do this the execute flags must be changed and this
  // must be done prior to this object being added to the warehouse in FEProblemBase::addTransfer.
  // It might be possible to create an Action that inspects the AddTransferActions, but this
  // solution is much simpler.
  if (_batch_mode)
  {
    ExecFlagEnum & flags = const_cast<ExecFlagEnum &>(getParam<ExecFlagEnum>("execute_on"));
    flags = {StochasticTools::EXEC_PRE_BATCH_MULTIAPP,
             StochasticTools::EXEC_BATCH_MULTIAPP,
             StochasticTools::EXEC_POST_BATCH_MULTIAPP};
  }
}

void
SamplerPostprocessorTransfer::initialSetup()
{
  auto & uo = _fe_problem.getUserObject<UserObject>(_master_vpp_name);
  _results = dynamic_cast<StochasticResults *>(&uo);

  if (!_results)
    mooseError("The 'results' object must be a 'StochasticResults' object.");

  _results->init(*_sampler);
}

void
SamplerPostprocessorTransfer::executeFromMultiapp()
{
  if (_batch_mode)
    executeFromMultiappBatch();
  else
    executeFromMultiappNormal();
}

void
SamplerPostprocessorTransfer::executeFromMultiappBatch()
{
  const ExecFlagType & exec_flag = _fe_problem.getCurrentExecuteOnFlag();

  // Initialize the storage for the PP values computed on this processor
  if (exec_flag == StochasticTools::EXEC_PRE_BATCH_MULTIAPP)
    _local_values.resize(0);

  // Collect the PP values for this processor
  else if (exec_flag == StochasticTools::EXEC_BATCH_MULTIAPP)
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

  else if (exec_flag == StochasticTools::EXEC_POST_BATCH_MULTIAPP)
  {
    // Gather the PP values from all ranks
    _communicator.allgather(_local_values);
    std::cerr << "_local_values.size() = " << _local_values.size() << std::endl;

    // Update VPP
    const dof_id_type n = _sampler->getTotalNumberOfRows();
    for (MooseIndex(n) i = 0; i < n; i++)
    {
      Sampler::Location loc = _sampler->getLocation(i);
      VectorPostprocessorValue & vpp = _results->getVectorPostprocessorValueByGroup(loc.sample());
      vpp[loc.row()] = _local_values[i];
    }
  }
}

void
SamplerPostprocessorTransfer::executeFromMultiappNormal()
{
  // Number of PP is equal to the number of MultiApps
  const unsigned int n = _multi_app->numGlobalApps();

  // Collect the PP values for this processor
  _local_values.resize(n);
  std::fill(_local_values.begin(), _local_values.end(), 0);
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

  // Update VPP
  for (unsigned int i = 0; i < n; i++)
  {
    Sampler::Location loc = _sampler->getLocation(i);
    VectorPostprocessorValue & vpp = _results->getVectorPostprocessorValueByGroup(loc.sample());
    vpp[loc.row()] = _local_values[i];
  }
}
