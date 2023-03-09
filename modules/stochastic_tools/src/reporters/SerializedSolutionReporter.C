//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SerializedSolutionReporter.h"
#include "NonlinearSystemBase.h"

registerMooseObject("StochasticToolsApp", SerializedSolutionReporter);

InputParameters
SerializedSolutionReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Something.");
  params.addRequiredParam<std::vector<VariableName>>("variables", "The variables to fetch.");
  return params;
}

SerializedSolutionReporter::SerializedSolutionReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _accumulated_solutions(declareRestartableData<
                           std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>>>(
        "accumulated_solution")),
    _variable_names(getParam<std::vector<VariableName>>("variables"))
{
  for (const auto & variable_name : _variable_names)
  {
    if (!_fe_problem.hasVariable(variable_name))
      paramError("variables", "Variable ( ", variable_name, ") does not exist in the problem!");
    else
      _accumulated_solutions[variable_name];
  }
}

void
SerializedSolutionReporter::initialSetup()
{
  _accumulated_solutions.clear();
}

void
SerializedSolutionReporter::execute()
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  PetscVector<Number> & solution = dynamic_cast<PetscVector<Number> &>(nl.solution());

  // Looping over the variables to extract the corresponding solution values
  for (const auto & variable_name : _variable_names)
  {
    // Getting the corresponding DoF indices for the variable. This needs to be a parallel call
    // to ensure the indices on all processors are considered.
    nl.setVariableGlobalDoFs(variable_name);

    // Initializing a temporary vector for the partial solution.
    std::unique_ptr<std::vector<Real>> serialized_solution = std::make_unique<std::vector<Real>>();

    // We only need this indices on the ROOT processor (where we will serialize). This
    // is essential for the localize function in libmesh.
    solution.localize(*serialized_solution,
                      processor_id() == 0 ? nl.getVariableGlobalDoFs()
                                          : std::vector<dof_id_type>());

    // Copying the temporary vector into the storage space. We will avoid doing anything
    // on processors which are not ROOT
    if (processor_id() == 0)
      _accumulated_solutions[variable_name].push_back(std::move(serialized_solution));
  }
}

void
dataStore(std::ostream & stream,
          std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>> & data,
          void * context)
{
  // First store the size of the map
  unsigned int size = data.size();
  stream.write((char *)&size, sizeof(size));

  typename std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>>::iterator it =
      data.begin();
  typename std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>>::iterator end =
      data.end();

  for (; it != end; ++it)
  {
    VariableName & key = const_cast<VariableName &>(it->first);

    storeHelper(stream, key, context);
    storeHelper(stream, it->second, context);
  }
}

void
dataLoad(std::istream & stream,
         std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>> & data,
         void * context)
{
  data.clear();

  // First read the size of the map
  unsigned int size = 0;
  stream.read((char *)&size, sizeof(size));

  for (unsigned int i = 0; i < size; i++)
  {
    VariableName key;
    loadHelper(stream, key, context);

    std::vector<std::unique_ptr<std::vector<Real>>> & value = data[key];
    loadHelper(stream, value, context);
  }
}
