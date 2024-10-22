//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LinearPicardSteady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "LinearSystem.h"
#include "libmesh/linear_implicit_system.h"

#include "libmesh/equation_systems.h"

registerMooseObject("MooseTestApp", LinearPicardSteady);

InputParameters
LinearPicardSteady::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addRequiredParam<std::vector<LinearSystemName>>(
      "linear_systems_to_solve",
      "The names of linear systems to solve in the order which they should be solved");
  params.addRangeCheckedParam<unsigned int>("number_of_iterations",
                                            1,
                                            "number_of_iterations>0",
                                            "The number of iterations between the two systems.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "petsc_options", {}, "Singleton PETSc options for each linear equation");
  params.addParam<std::vector<std::vector<std::string>>>(
      "petsc_options_iname", {}, "Names of PETSc name/value pairs for each linear equation");
  params.addParam<std::vector<std::vector<std::string>>>(
      "petsc_options_value",
      {},
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for each "
      "linear equation");

  params.addParam<bool>(
      "print_operators_and_vectors",
      false,
      "Print system matrix, right hand side and solution for the linear systems.");

  return params;
}

LinearPicardSteady::LinearPicardSteady(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _time_step(_problem.timeStep()),
    _time(_problem.time()),
    _linear_sys_names(getParam<std::vector<LinearSystemName>>("linear_systems_to_solve")),
    _petsc_options(_linear_sys_names.size()),
    _number_of_iterations(getParam<unsigned int>("number_of_iterations")),
    _print_operators_and_vectors(getParam<bool>("print_operators_and_vectors"))
{
  const auto & raw_petsc_options = getParam<std::vector<std::vector<std::string>>>("petsc_options");
  const auto & raw_petsc_options_iname =
      getParam<std::vector<std::vector<std::string>>>("petsc_options_iname");
  const auto & raw_petsc_options_value =
      getParam<std::vector<std::vector<std::string>>>("petsc_options_value");

  if (raw_petsc_options.size() > 1 && raw_petsc_options.size() != _linear_sys_numbers.size())
    paramError("petsc_options", "Petsc options should be defined for every system separately!");

  if (raw_petsc_options_iname.size() > 1 &&
      raw_petsc_options_iname.size() != _linear_sys_numbers.size())
    paramError("petsc_options_iname",
               "Petsc option keys should be defined for every system separately!");

  if (raw_petsc_options_value.size() != raw_petsc_options_iname.size())
    paramError(
        "petsc_options_value",
        "Petsc option values should be defined for the same number of system as in the keys!");

  for (const auto i : index_range(_linear_sys_names))
  {
    _linear_sys_numbers.push_back(_problem.linearSysNum(_linear_sys_names[i]));

    MultiMooseEnum enum_singles = Moose::PetscSupport::getCommonPetscFlags();

    if (raw_petsc_options.size() == 1)
      enum_singles = raw_petsc_options[0];
    else if (raw_petsc_options.size() > 1)
      enum_singles = raw_petsc_options[i];

    MultiMooseEnum enum_pair_keys = Moose::PetscSupport::getCommonPetscKeys();
    if (raw_petsc_options_iname.size() == 1)
      enum_pair_keys = raw_petsc_options_iname[0];
    else if (raw_petsc_options_iname.size() > 1)
      enum_pair_keys = raw_petsc_options_iname[i];

    std::vector<std::string> enum_pair_values;
    if (raw_petsc_options_value.size() == 1)
      enum_pair_values = raw_petsc_options_value[0];
    else if (raw_petsc_options_value.size() > 1)
      enum_pair_values = raw_petsc_options_value[i];

    if (enum_pair_keys.size() != enum_pair_values.size())
      paramError("petsc_options_value",
                 "The size of petsc option values for system " + _linear_sys_names[i] + " (" +
                     std::to_string(enum_pair_keys.size()) +
                     ") does not match the size of the input arguments (" +
                     std::to_string(enum_pair_values.size()) + ")!");

    std::vector<std::pair<MooseEnumItem, std::string>> raw_iname_value_pairs;
    for (const auto j : index_range(enum_pair_values))
      raw_iname_value_pairs.push_back(std::make_pair(enum_pair_keys[j], enum_pair_values[j]));

    Moose::PetscSupport::processPetscFlags(enum_singles, _petsc_options[i]);
    Moose::PetscSupport::processPetscPairs(
        raw_iname_value_pairs, _problem.mesh().dimension(), _petsc_options[i]);
  }

  _time = 0;
}

void
LinearPicardSteady::init()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    return;
  }

  _problem.initialSetup();
}

void
LinearPicardSteady::execute()
{
  if (_app.isRecovering())
    return;

  _time_step = 0;
  _problem.outputStep(EXEC_INITIAL);

  preExecute();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

  _problem.timestepSetup();

  preSolve();
  _problem.execute(EXEC_TIMESTEP_BEGIN);
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);
  _problem.updateActiveObjects();

  for (unsigned int i = 0; i < _number_of_iterations; i++)
    for (const auto sys_index : index_range(_linear_sys_numbers))
    {
      const auto & options = _petsc_options[sys_index];
      solveSystem(_linear_sys_numbers[sys_index], &options);
    }

  for (const auto sys_number : _linear_sys_numbers)
    _problem.getLinearSystem(sys_number).computeGradients();

  _last_solve_converged = true;

  // need to keep _time in sync with _time_step to get correct output
  _time = _time_step;
  _problem.execute(EXEC_TIMESTEP_END);
  _problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.postExecute();
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
  }

  postExecute();
}

bool
LinearPicardSteady::solveSystem(const unsigned int sys_number,
                                const Moose::PetscSupport::PetscOptions * po)
{
  _problem.solveLinearSystem(sys_number, po);
  if (_print_operators_and_vectors)
  {
    auto & sys = _problem.getLinearSystem(sys_number);
    LinearImplicitSystem & lisystem = libMesh::cast_ref<LinearImplicitSystem &>(sys.system());
    lisystem.matrix->print();
    lisystem.rhs->print();
    lisystem.solution->print();
  }

  return true;
}
