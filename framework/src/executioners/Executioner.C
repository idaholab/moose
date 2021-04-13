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

// C++ includes
#include <vector>
#include <limits>

defineLegacyParams(Executioner);

InputParameters
Executioner::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += FEProblemSolve::validParams();
  params += PicardSolve::validParams();
  params += Reporter::validParams();
  params += ReporterInterface::validParams();

  params.addDeprecatedParam<FileNameNoExtension>(
      "restart_file_base",
      "",
      "File base name used for restart",
      "Please use \"Problem/restart_file_base\" instead");

  params.registerBase("Executioner");

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
    _feproblem_solve(this),
    _picard_solve(this),
    _restart_file_base(getParam<FileNameNoExtension>("restart_file_base")),
    _verbose(getParam<bool>("verbose")),
    _num_grid_steps(getParam<unsigned int>("num_grids") - 1)
{
  if (!_restart_file_base.empty())
    _fe_problem.setRestartFile(_restart_file_base);

  auto & nl = _fe_problem.getNonlinearSystemBase();

  // Check whether the user has explicitly requested automatic scaling and is using a solve type
  // without a matrix. If so, then we warn them
  if ((_pars.isParamSetByUser("automatic_scaling") && getParam<bool>("automatic_scaling")) &&
      _fe_problem.solverParams()._type == Moose::ST_JFNK)
  {
    paramWarning("automatic_scaling",
                 "Automatic scaling isn't implemented for the case where you do not have a "
                 "preconditioning matrix. No scaling will be applied");
    _fe_problem.automaticScaling(false);
  }
  else
    // Check to see whether automatic_scaling has been specified anywhere, including at the
    // application level. No matter what: if we don't have a matrix, we don't do scaling
    _fe_problem.automaticScaling((isParamValid("automatic_scaling")
                                      ? getParam<bool>("automatic_scaling")
                                      : getMooseApp().defaultAutomaticScaling()) &&
                                 (_fe_problem.solverParams()._type != Moose::ST_JFNK));

  nl.computeScalingOnce(getParam<bool>("compute_scaling_once"));
  nl.autoScalingParam(getParam<Real>("resid_vs_jac_scaling_param"));
  if (isParamValid("scaling_group_variables"))
    nl.scalingGroupVariables(
        getParam<std::vector<std::vector<std::string>>>("scaling_group_variables"));

  _fe_problem.numGridSteps(_num_grid_steps);
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
