//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "ResidualTransfer.h"
#include "NonlinearSystemBase.h"

registerMooseObject("StochasticToolsApp", ResidualTransfer);

InputParameters
ResidualTransfer::validParams()
{
  InputParameters params = SamplerSolutionTransfer::validParams();
  params.addClassDescription("Transfers residual vectors from the sub-application to a "
                             "a container in the Trainer object.");
  return params;
}

ResidualTransfer::ResidualTransfer(const InputParameters & parameters)
  : SamplerSolutionTransfer(parameters)
{}

void
ResidualTransfer::initialSetup()
{
  SamplerSolutionTransfer::initialSetup();
}

void
ResidualTransfer::initializeFromMultiapp()
{}

void
ResidualTransfer::executeFromMultiapp()
{}

void
ResidualTransfer::finalizeFromMultiapp()
{}

void
ResidualTransfer::execute()
{
  const std::vector<std::string>& var_names = _trainer->getVarNames();
  const std::vector<std::string>& tag_names = _trainer->getTagNames();
  const unsigned int total_base_num = _trainer->getSumBaseSize();

  switch(_direction)
  {
    case FROM_MULTIAPP:

      _trainer->initReducedOperators();

      // Looping over sub-apps
      for (unsigned int base_i = 0; base_i < total_base_num; ++base_i)
      {
        // Getting reference to solution vector
        FEProblemBase & app_problem = _multi_app->appProblemBase(base_i);
        NonlinearSystemBase& nl = app_problem.getNonlinearSystemBase();

        for (unsigned int tag_i = 0; tag_i < tag_names.size(); ++tag_i)
        {
          TagID tag_id = app_problem.getVectorTagID(tag_names[tag_i]);

          NumericVector<Number>& full_residual = nl.getVector(tag_id);
          std::vector<DenseVector<Real>> split_residual(var_names.size());

          for (unsigned int var_i=0; var_i<var_names.size(); ++var_i)
          {
            nl.setVariableGlobalDoFs(var_names[var_i]);
            const std::vector<dof_id_type>& var_dofs = nl.getVariableGlobalDoFs();

            full_residual.get(var_dofs, split_residual[var_i].get_values());

            _trainer->addToReducedOperator(base_i, tag_i, split_residual);
          }
        }
      }
      break;

    case TO_MULTIAPP:

      std::cout << "Nothing yet!" << std::endl;

      break;
  }

}
