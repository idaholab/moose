//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SerializedSolutionTransfer.h"
#include "NonlinearSystemBase.h"
#include "Sampler.h"
#include "VectorPacker.h"

registerMooseObject("StochasticToolsApp", SerializedSolutionTransfer);

InputParameters
SerializedSolutionTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Noice.");
  params.addRequiredParam<std::string>("parallel_storage_name", "Something here.");
  params.addRequiredParam<std::string>("serialized_solution_reporter", "Something here.");
  params.addRequiredParam<std::vector<VariableName>>("variables", "Something.");
  return params;
}

SerializedSolutionTransfer::SerializedSolutionTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _variable_names(getParam<std::vector<VariableName>>("variables")),
    _serialized_solution_reporter(getParam<std::string>("serialized_solution_reporter")),
    _num_global_entries(0),
    _num_local_entries(0)
{
}

void
SerializedSolutionTransfer::initialSetup()
{
  std::string parallel_storage_name = getParam<std::string>("parallel_storage_name");

  std::vector<UserObject *> reporters;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(parallel_storage_name)
      .queryInto(reporters);

  if (reporters.empty())
    paramError(
        "parallel_storage_name", "Unable to find reporter with name '", parallel_storage_name, "'");
  else if (reporters.size() > 1)
    paramError("parallel_storage_name",
               "We found more than one reporter with the name '",
               parallel_storage_name,
               "'");

  _parallel_storage = dynamic_cast<ParallelSolutionStorage *>(reporters[0]);

  if (!_parallel_storage)
    paramError("parallel_storage_name",
               "The parallel storage reporter is not of type '",
               parallel_storage_name,
               "'");
}

void
SerializedSolutionTransfer::execute()
{
}

void
SerializedSolutionTransfer::initializeFromMultiapp()
{
}

void
SerializedSolutionTransfer::executeFromMultiapp()
{
  unsigned int num_incoming_local_entries = 0;
  unsigned int num_incoming_global_entries = 0;

  // Getting the reference to the solution vector in the subapp.
  FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(_app_index);

  std::vector<UserObject *> reporters;
  app_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(_serialized_solution_reporter)
      .queryInto(reporters);

  if (reporters.empty())
    paramError("serialized_solution_reporter",
               "Unable to find reporter with name '",
               _serialized_solution_reporter,
               "'");
  else if (reporters.size() > 1)
    paramError("serialized_solution_reporter",
               "We found more than one reporter with the name '",
               _serialized_solution_reporter,
               "'");

  SolutionContainer * solution_container = dynamic_cast<SolutionContainer *>(reporters[0]);

  if (!solution_container)
    paramError("serialized_solution_reporter",
               "The parallel storage reporter is not of type '",
               _serialized_solution_reporter,
               "'");

  if (getFromMultiApp()->hasLocalApp(_app_index))
  {
    FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(_app_index);
    NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();

    // Looping over the variables to extract the corresponding solution values
    // and copy them into the container of the trainer.
    for (unsigned int var_i = 0; var_i < _variable_names.size(); ++var_i)
    {
      // Getting the corresponding DoF indices for the variable.
      nl.setVariableGlobalDoFs(_variable_names[var_i]);
      const std::vector<dof_id_type> & var_dofs = nl.getVariableGlobalDoFs();

      for (unsigned int solution_i = 0; solution_i < solution_container->getContainer().size();
           ++solution_i)
      {
        std::unique_ptr<DenseVector<Real>> serialized_solution =
            std::make_unique<DenseVector<Real>>();
        solution_container->getSolution(solution_i)
            ->localize(serialized_solution->get_values(),
                       getFromMultiApp()->isRootProcessor() ? nl.getVariableGlobalDoFs()
                                                            : std::vector<dof_id_type>());

        if (getFromMultiApp()->isRootProcessor())
        {
          _parallel_storage->addEntry(
              _variable_names[var_i], _global_index, std::move(serialized_solution));
        }
      }
    }
  }

  _parallel_storage->printEntries();
}

void
SerializedSolutionTransfer::finalizeFromMultiapp()
{
}

void
SerializedSolutionTransfer::initializeToMultiapp()
{
}

void
SerializedSolutionTransfer::executeToMultiapp()
{
}

void
SerializedSolutionTransfer::finalizeToMultiapp()
{
}
