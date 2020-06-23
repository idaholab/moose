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
  return params;
}

SamplerSolutionTransfer::SamplerSolutionTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
  _trainer_name(getParam<std::string>("trainer_name"))
{
  std::vector<PODReducedBasisTrainer *> obj;

  _fe_problem.theWarehouse()
               .query()
               .condition<AttribName>(_trainer_name)
               .queryInto(obj);

  if (obj.empty())
    mooseError("Unable to find Trainer with name '"+ _trainer_name + "'!");

  _trainer = obj[0];
}

void
SamplerSolutionTransfer::initialSetup()
{
  // Checking if the subapplication has the requested variables
  const std::vector<std::string>& var_names = _trainer->varNames();
  const dof_id_type n = _multi_app->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
  {
    if (_multi_app->hasLocalApp(i))
      for(auto var_name : var_names)
        if (!_multi_app->appProblemBase(i).hasVariable(var_name))
          mooseError("Variable '"+var_name+"' not found in the sub-application!");
  }
}

void
SamplerSolutionTransfer::initializeFromMultiapp()
{
}

void
SamplerSolutionTransfer::executeFromMultiapp()
{
  // const dof_id_type n = _multi_app->numGlobalApps();
  // for (MooseIndex(n) i = 0; i < n; i++)
  // {
  //   if (_multi_app->hasLocalApp(i))
  //   {
  //     FEProblemBase & app_problem = _multi_app->appProblemBase(i);
  //
  //     NumericVector<Number>& solution = app_problem.getNonlinearSystemBase().solution();
  //
  //     auto& subproblem = app_problem.getNonlinearSystemBase().subproblem();
  //
  //     std::vector<VariableName> asd = app_problem.getVariableNames();
  //
  //     for (unsigned i=0; i< asd.size(); i++)
  //     {
  //       std::cout << asd[i] << std::endl;
  //     }
  //
  //     for(auto it=app_problem._var_dof_map.begin(); it != app_problem._var_dof_map.end(); ++it)
  //       std::cout << it->first << std::endl;
  //
  //     _trainer->addSnapshot(solution, app_problem._var_dof_map);
  //   }
  // }
}

void
SamplerSolutionTransfer::finalizeFromMultiapp()
{
}

void
SamplerSolutionTransfer::execute()
{
  const std::vector<std::string>& var_names = _trainer->varNames();

  switch(_direction)
  {
    case FROM_MULTIAPP:
      // Looping over sub-apps
      for (dof_id_type i = _sampler_ptr->getLocalRowBegin(); i < _sampler_ptr->getLocalRowEnd(); ++i)
      {
        // Getting reference to solution vector
        FEProblemBase & app_problem = _multi_app->appProblemBase(i);
        NonlinearSystemBase& nl = app_problem.getNonlinearSystemBase();
        NumericVector<Number>& solution = nl.solution();

        for (auto var_name : var_names)
        {
          nl.setVariableGlobalDoFs(var_name);
          const std::vector<dof_id_type>& var_dofs = nl.getVariableGlobalDoFs();

          DenseVector<Real> tmp;
          solution.get(var_dofs, tmp.get_values());

          _trainer->addSnapshot(var_name, tmp);
        }
      }
      break;

    case TO_MULTIAPP:

      unsigned int counter = 0;

      for (auto var_name : var_names)
      {
        unsigned int var_base_num = _trainer->getBaseSize(var_name);

        for(unsigned int base_i=0; base_i<var_base_num; ++base_i)
        {
          FEProblemBase & app_problem = _multi_app->appProblemBase(counter);
          NonlinearSystemBase& nl = app_problem.getNonlinearSystemBase();
          NumericVector<Number>& solution = nl.solution();

          solution.zero();

          nl.setVariableGlobalDoFs(var_name);
          const std::vector<dof_id_type>& var_dofs = nl.getVariableGlobalDoFs();

          const DenseVector<Real>& base_vector = _trainer->getBasisVector(var_name, base_i);

          solution.insert(base_vector, var_dofs);

          solution.close();

          counter++;
        }
      }
      break;
  }

}
