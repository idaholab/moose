//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "PODSamplerSolutionTransfer.h"
#include "NonlinearSystemBase.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", PODSamplerSolutionTransfer);

InputParameters
PODSamplerSolutionTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Transfers solution vectors from the sub-applications to a "
                             "a container in the Trainer object and back.");
  params.addRequiredParam<UserObjectName>("trainer_name",
                                          "Trainer object that contains the solutions"
                                          " for different samples.");
  return params;
}

PODSamplerSolutionTransfer::PODSamplerSolutionTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    SurrogateModelInterface(this),
    _pod_multi_app(hasFromMultiApp()
                       ? std::dynamic_pointer_cast<PODFullSolveMultiApp>(getFromMultiApp())
                       : std::dynamic_pointer_cast<PODFullSolveMultiApp>(getToMultiApp())),
    _trainer(getSurrogateTrainer<PODReducedBasisTrainer>("trainer_name"))
{
  if (!_pod_multi_app)
    paramError("multi_app", "The Multiapp given is not a PODFullsolveMultiapp!");
}

void
PODSamplerSolutionTransfer::initialSetup()
{
  const auto multi_app = hasFromMultiApp() ? getFromMultiApp() : getToMultiApp();

  // Checking if the subapplication has the requested variables
  const std::vector<std::string> & var_names = _trainer.getVarNames();
  const dof_id_type n = multi_app->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
  {
    if (multi_app->hasLocalApp(i))
      for (auto var_name : var_names)
        if (!multi_app->appProblemBase(i).hasVariable(var_name))
          mooseError("Variable '" + var_name + "' not found on sub-application ", i, "!");
  }
}

void
PODSamplerSolutionTransfer::execute()
{

  const std::vector<std::string> & var_names = _trainer.getVarNames();

  // Selecting the appropriate action based on the drection.
  switch (_direction)
  {
    case FROM_MULTIAPP:

      // Looping over sub-apps created for different samples
      for (dof_id_type i = _sampler_ptr->getLocalRowBegin(); i < _sampler_ptr->getLocalRowEnd();
           ++i)
      {
        // Getting reference to the  solution vector of the sub-app.
        FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(i);
        NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();
        NumericVector<Number> & solution = nl.solution();

        // Looping over the variables to extract the corresponding solution values
        // and copy them into the container of the trainer.
        for (unsigned int v_index = 0; v_index < var_names.size(); ++v_index)
        {
          // Getting the corresponding DoF indices for the variable.
          nl.setVariableGlobalDoFs(var_names[v_index]);
          const std::vector<dof_id_type> & var_dofs = nl.getVariableGlobalDoFs();

          // Initializing a temporary vector for the partial solution.
          std::shared_ptr<DenseVector<Real>> tmp = std::make_shared<DenseVector<Real>>();
          solution.get(var_dofs, tmp->get_values());

          // Copying the temporary vector into the trainer.
          _trainer.addSnapshot(v_index, i, tmp);
        }
      }
      break;

    case TO_MULTIAPP:

      // Looping over all the variables in the trainer to copy the corresponding
      // basis vectors into the solution.
      unsigned int counter = 0;
      for (unsigned int var_i = 0; var_i < var_names.size(); ++var_i)
      {
        // Looping over the bases of the given variable and plugging them into
        // a sub-application.
        unsigned int var_base_num = _trainer.getBaseSize(var_i);
        for (unsigned int base_i = 0; base_i < var_base_num; ++base_i)
        {
          if (getToMultiApp()->hasLocalApp(counter))
          {
            // Getting the reference to the solution vector in the subapp.
            FEProblemBase & app_problem = getToMultiApp()->appProblemBase(counter);
            NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();
            NumericVector<Number> & solution = nl.solution();

            // Zeroing the solution to make sure that only the required part
            // is non-zero after copy.
            solution.zero();

            // Getting the degrees of freedom for the given variable.
            nl.setVariableGlobalDoFs(var_names[var_i]);
            const std::vector<dof_id_type> & var_dofs = nl.getVariableGlobalDoFs();

            // Fetching the basis vector and plugging it into the solution.
            const DenseVector<Real> & base_vector = _trainer.getBasisVector(var_i, base_i);
            solution.insert(base_vector, var_dofs);
            solution.close();

            // Make sure that the sub-application uses this vector to evaluate the
            // residual.
            nl.setSolution(solution);
          }
          counter++;
        }
      }
      break;
  }
}

void
PODSamplerSolutionTransfer::initializeFromMultiapp()
{
}

void
PODSamplerSolutionTransfer::executeFromMultiapp()
{
  if (_pod_multi_app->snapshotGeneration())
  {
    const std::vector<std::string> & var_names = _trainer.getVarNames();

    const dof_id_type n = getFromMultiApp()->numGlobalApps();

    for (MooseIndex(n) i = 0; i < n; i++)
    {
      if (getFromMultiApp()->hasLocalApp(i))
      {
        // Getting reference to the  solution vector of the sub-app.
        FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(i);
        NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();
        NumericVector<Number> & solution = nl.solution();

        // Looping over the variables to extract the corresponding solution values
        // and copy them into the container of the trainer.
        for (unsigned int var_i = 0; var_i < var_names.size(); ++var_i)
        {
          // Getting the corresponding DoF indices for the variable.
          nl.setVariableGlobalDoFs(var_names[var_i]);
          const std::vector<dof_id_type> & var_dofs = nl.getVariableGlobalDoFs();

          // Initializing a temporary vector for the partial solution.
          std::shared_ptr<DenseVector<Real>> tmp = std::make_shared<DenseVector<Real>>();
          solution.get(var_dofs, tmp->get_values());

          // Copying the temporary vector into the trainer.
          _trainer.addSnapshot(var_i, _global_index, tmp);
        }
      }
    }
  }
}

void
PODSamplerSolutionTransfer::finalizeFromMultiapp()
{
}

void
PODSamplerSolutionTransfer::initializeToMultiapp()
{
}

void
PODSamplerSolutionTransfer::executeToMultiapp()
{
  if (!_pod_multi_app->snapshotGeneration())
  {
    const std::vector<std::string> & var_names = _trainer.getVarNames();
    dof_id_type var_i = _trainer.getVariableIndex(_global_index);

    // Getting the reference to the solution vector in the subapp.
    FEProblemBase & app_problem = getToMultiApp()->appProblemBase(processor_id());
    NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();
    NumericVector<Number> & solution = nl.solution();

    // Zeroing the solution to make sure that only the required part
    // is non-zero after copy.
    solution.zero();

    // Getting the degrees of freedom for the given variable.
    nl.setVariableGlobalDoFs(var_names[var_i]);
    const std::vector<dof_id_type> & var_dofs = nl.getVariableGlobalDoFs();

    // Fetching the basis vector and plugging it into the solution.
    const DenseVector<Real> & base_vector = _trainer.getBasisVector(_global_index);
    solution.insert(base_vector, var_dofs);
    solution.close();

    // Make sure that the sub-application uses this vector to evaluate the
    // residual.
    nl.setSolution(solution);
  }
}

void
PODSamplerSolutionTransfer::finalizeToMultiapp()
{
}
