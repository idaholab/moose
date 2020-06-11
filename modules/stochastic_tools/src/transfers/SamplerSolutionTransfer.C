//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SamplerSolutionTransfer.h"
#include "SamplerFullSolveMultiApp.h"
#include "SamplerTransientMultiApp.h"
#include "NonlinearSystemBase.h"
#include "SamplerReceiver.h"
#include "StochasticResults.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", SamplerSolutionTransfer);

InputParameters
SamplerSolutionTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Transfers solution vectors from the sub-application to a "
                             "a container in the Transfer object.");
  params.addRequiredParam<std::string>("trainer_name", "Trainer object that contains the solutions for different samples.");

  params.set<MultiMooseEnum>("direction") = "from_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");
  return params;
}

SamplerSolutionTransfer::SamplerSolutionTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
  _trainer_name(getParam<std::string>("trainer_name"))
{}

void
SamplerSolutionTransfer::initialSetup()
{
  FEProblemBase & problem = *(this->parameters().get<FEProblemBase *>("_fe_problem_base"));
  std::vector<PODReducedBasisTrainer *> obj;

  problem.theWarehouse()
             .query()
             .condition<AttribName>(_trainer_name)
             .queryInto(obj);

  if (obj.empty())
    mooseError("Unable to find Trainer with name '"+ _trainer_name + "'!");

  _trainer = obj[0];
}

void
SamplerSolutionTransfer::initializeFromMultiapp()
{
}

void
SamplerSolutionTransfer::executeFromMultiapp()
{
  const dof_id_type n = _multi_app->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
  {
    if (_multi_app->hasLocalApp(i))
    {
      FEProblemBase & app_problem = _multi_app->appProblemBase(i);

      NumericVector<Number>& solution = app_problem.getNonlinearSystemBase().solution();

      _trainer->addSnapshot(solution);
    }
  }
}

void
SamplerSolutionTransfer::finalizeFromMultiapp()
{
}

void
SamplerSolutionTransfer::execute()
{
  for (dof_id_type i = _sampler_ptr->getLocalRowBegin(); i < _sampler_ptr->getLocalRowEnd(); ++i)
  {
    FEProblemBase & app_problem = _multi_app->appProblemBase(i);

    NumericVector<Number>& solution = app_problem.getNonlinearSystemBase().solution();

    _trainer->addSnapshot(solution);
  }
}
