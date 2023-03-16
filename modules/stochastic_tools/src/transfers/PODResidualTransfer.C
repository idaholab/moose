//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "PODResidualTransfer.h"
#include "NonlinearSystemBase.h"

registerMooseObject("StochasticToolsApp", PODResidualTransfer);

InputParameters
PODResidualTransfer::validParams()
{
  InputParameters params = PODSamplerSolutionTransfer::validParams();
  params.addClassDescription("Transfers residual vectors from the sub-application to a "
                             "a container in the Trainer object.");
  params.suppressParameter<MultiMooseEnum>("direction");
  params.suppressParameter<MultiAppName>("multi_app");
  return params;
}

PODResidualTransfer::PODResidualTransfer(const InputParameters & parameters)
  : PODSamplerSolutionTransfer(parameters)
{
  if (hasToMultiApp())
    paramError("to_multi_app", "To and between multiapp directions are not implemented");
}

void
PODResidualTransfer::execute()
{
  const unsigned int total_base_num = _trainer.getSumBaseSize();

  // Looping over sub-apps
  for (unsigned int base_i = 0; base_i < total_base_num; ++base_i)
    if (getFromMultiApp()->hasLocalApp(base_i))
      transferResidual(base_i, base_i);
}

void
PODResidualTransfer::executeFromMultiapp()
{
  transferResidual(_global_index, processor_id());
}

void
PODResidualTransfer::transferResidual(dof_id_type base_i, dof_id_type multi_app_i)
{
  const std::vector<std::string> & var_names = _trainer.getVarNames();
  const std::vector<std::string> & tag_names = _trainer.getTagNames();
  const std::vector<std::string> & tag_types = _trainer.getTagTypes();

  // Getting reference to the non-linear system
  FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(multi_app_i);
  NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();

  // Looping over the residual tags and extracting the corresponding vector.
  for (unsigned int tag_i = 0; tag_i < tag_names.size(); ++tag_i)
  {
    // If the tag corresponds to an independent operator, it is enough to
    // transfer it once.
    if (base_i > 0 && (tag_types[tag_i] == "src" || tag_types[tag_i] == "src_dir"))
      continue;

    TagID tag_id = app_problem.getVectorTagID(tag_names[tag_i]);

    // Fetching the corresponding residual vector and extracting the parts for
    // each variable.
    NumericVector<Number> & full_residual = nl.getVector(tag_id);
    std::vector<DenseVector<Real>> split_residual(var_names.size());

    for (unsigned int var_i = 0; var_i < var_names.size(); ++var_i)
    {
      // Getting the DoF indices of the variable.
      nl.setVariableGlobalDoFs(var_names[var_i]);
      const std::vector<dof_id_type> & var_dofs = nl.getVariableGlobalDoFs();

      // Extracting the corresponding part of the residual vector.
      full_residual.get(var_dofs, split_residual[var_i].get_values());
    }

    // Inserting the contribution of this residual into the reduced operator in
    // the trainer.
    _trainer.addToReducedOperator(base_i, tag_i, split_residual);
  }
}
