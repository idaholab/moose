//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "Executioner.h"

#include "MooseApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "SlepcSupport.h"
#include "SecantSolve.h"
#include "SteffensenSolve.h"
#include "Factory.h"

// C++ includes
#include <vector>
#include <limits>

defineLegacyParams(Executioner);

InputParameters
Executioner::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addClassDescription("The executioner that all other executioners are derived from "
                             "provides management of all solve objects.");
  params += Reporter::validParams();
  params += ReporterInterface::validParams();
  params.registerBase("Executioner");

  // we temporarily allow deprecated parameters for previous implementation of Picard iteration
  // This should be replaced with FixedPointSolve::validParams() in the future.
  params += PicardSolve::validParams();

  params.addParam<MooseEnum>("fixed_point_algorithm",
                             iterationMethods(),
                             "The fixed point algorithm to converge the sequence of problems.");

  params.addParam<bool>("verbose", false, "Set to true to print additional information");

  params.addDeprecatedParam<FileNameNoExtension>(
      "restart_file_base",
      "",
      "File base name used for restart",
      "Please use \"Problem/restart_file_base\" instead");

  params.addParamNamesToGroup("restart_file_base", "Restart");

  return params;
}

Executioner::Executioner(const InputParameters & parameters)
  : MooseObject(parameters),
    Reporter(this),
    ReporterInterface(this),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    Restartable(this, "Executioners"),
    PerfGraphInterface(this),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base", "This might happen if you don't have a mesh")),
    _iteration_method(getParam<MooseEnum>("fixed_point_algorithm")),
    _time_step(_fe_problem.timeStep()),
    _time(_fe_problem.time()),
    _restart_file_base(getParam<FileNameNoExtension>("restart_file_base")),
    _verbose(getParam<bool>("verbose"))
{
  if (!_restart_file_base.empty())
    _fe_problem.setRestartFile(_restart_file_base);

  // Instantiate the SolveObject for the fixed point iteration algorithm
  if (_iteration_method == "picard")
    _fixed_point_solve = addSolveObject<PicardSolve>();
  else if (_iteration_method == "secant")
    _fixed_point_solve = addSolveObject<SecantSolve>();
  else if (_iteration_method == "steffensen")
    _fixed_point_solve = addSolveObject<SteffensenSolve>();
}

Problem &
Executioner::problem()
{
  mooseDoOnce(mooseWarning("This method is deprecated, use feProblem() instead"));
  return _fe_problem;
}

FEProblemBase &
Executioner::feProblem()
{
  return _fe_problem;
}

PostprocessorValue &
Executioner::addAttributeReporter(const std::string & name, Real initial_value)
{
  // Get a reference to the value
  PostprocessorValue & value = declareValueByName<PostprocessorValue>(name, initial_value);

  // Create storage for the old/older values
  ReporterName r_name(this->name(), name);
  getReporterValueByName<PostprocessorValue>(r_name, 1);
  getReporterValueByName<PostprocessorValue>(r_name, 2);

  return value;
}

void
Executioner::addSolveObject(const std::string & type,
                            const std::string & name,
                            InputParameters & parameters)
{
  if (_solve_object_names.count(type))
    mooseError("Solve object in type ", type, " has been constructed");

  parameters.set<SubProblem *>("_subproblem") = &feProblem();
  parameters.set<SystemBase *>("_sys") = &feProblem().getNonlinearSystemBase();
  parameters.set<bool>("verbose") = getParam<bool>("verbose");
  _solve_objects[name] = _app.getFactory().create<SolveObject>(type, name, parameters, 0);
  _solve_object_names[type] = name;
}

void
Executioner::init()
{
  checkIntegrity();
  _fe_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _fe_problem.initialSetup();

  for (auto & ptr : _self_solve_objects)
    ptr->init();

  for (auto & pair : _solve_objects)
    pair.second->init();
}
