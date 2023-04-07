//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblemSolve.h"

#include "FEProblem.h"
#include "NonlinearSystemBase.h"

std::set<std::string> const FEProblemSolve::_moose_line_searches = {"contact", "project"};

const std::set<std::string> &
FEProblemSolve::mooseLineSearches()
{
  return _moose_line_searches;
}

InputParameters
FEProblemSolve::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<std::vector<std::string>>("splitting",
                                            "Top-level splitting defining a "
                                            "hierarchical decomposition into "
                                            "subsystems to help the solver.");

  std::set<std::string> line_searches = mooseLineSearches();

  std::set<std::string> alias_line_searches = {"default", "none", "basic"};
  line_searches.insert(alias_line_searches.begin(), alias_line_searches.end());
  std::set<std::string> petsc_line_searches = Moose::PetscSupport::getPetscValidLineSearches();
  line_searches.insert(petsc_line_searches.begin(), petsc_line_searches.end());
  std::string line_search_string = Moose::stringify(line_searches, " ");
  MooseEnum line_search(line_search_string, "default");
  std::string addtl_doc_str(" (Note: none = basic)");
  params.addParam<MooseEnum>(
      "line_search", line_search, "Specifies the line search type" + addtl_doc_str);
  MooseEnum line_search_package("petsc moose", "petsc");
  params.addParam<MooseEnum>("line_search_package",
                             line_search_package,
                             "The solver package to use to conduct the line-search");

  params.addParam<unsigned>("contact_line_search_allowed_lambda_cuts",
                            2,
                            "The number of times lambda is allowed to be cut in half in the "
                            "contact line search. We recommend this number be roughly bounded by 0 "
                            "<= allowed_lambda_cuts <= 3");
  params.addParam<Real>("contact_line_search_ltol",
                        "The linear relative tolerance to be used while the contact state is "
                        "changing between non-linear iterations. We recommend that this tolerance "
                        "be looser than the standard linear tolerance");

  params += Moose::PetscSupport::getPetscValidParams();
  params.addParam<Real>("l_tol", 1.0e-5, "Linear Tolerance");
  params.addParam<Real>("l_abs_tol", 1.0e-50, "Linear Absolute Tolerance");
  params.addParam<unsigned int>("l_max_its", 10000, "Max Linear Iterations");
  params.addParam<unsigned int>("nl_max_its", 50, "Max Nonlinear Iterations");
  params.addParam<unsigned int>("nl_forced_its", 0, "The Number of Forced Nonlinear Iterations");
  params.addParam<unsigned int>("nl_max_funcs", 10000, "Max Nonlinear solver function evaluations");
  params.addParam<Real>("nl_abs_tol", 1.0e-50, "Nonlinear Absolute Tolerance");
  params.addParam<Real>("nl_rel_tol", 1.0e-8, "Nonlinear Relative Tolerance");
  params.addParam<Real>(
      "nl_div_tol",
      1.0e10,
      "Nonlinear Relative Divergence Tolerance. A negative value disables this check.");
  params.addParam<Real>(
      "nl_abs_div_tol",
      1.0e50,
      "Nonlinear Absolute Divergence Tolerance. A negative value disables this check.");
  params.addParam<Real>("nl_abs_step_tol", 0., "Nonlinear Absolute step Tolerance");
  params.addParam<Real>("nl_rel_step_tol", 0., "Nonlinear Relative step Tolerance");
  params.addParam<unsigned int>(
      "n_max_nonlinear_pingpong",
      100,
      "The maximum number of times the non linear residual can ping pong "
      "before requesting halting the current evaluation and requesting timestep cut");
  params.addParam<bool>(
      "snesmf_reuse_base",
      true,
      "Specifies whether or not to reuse the base vector for matrix-free calculation");
  params.addParam<bool>(
      "skip_exception_check", false, "Specifies whether or not to skip exception check");
  params.addParam<bool>(
      "compute_initial_residual_before_preset_bcs",
      false,
      "Use the residual norm computed *before* preset BCs are imposed in relative "
      "convergence check");
  params.addParam<bool>("automatic_scaling", "Whether to use automatic scaling for the variables.");
  params.addParam<bool>(
      "compute_scaling_once",
      true,
      "Whether the scaling factors should only be computed once at the beginning of the simulation "
      "through an extra Jacobian evaluation. If this is set to false, then the scaling factors "
      "will be computed during an extra Jacobian evaluation at the beginning of every time step.");
  params.addParam<bool>(
      "off_diagonals_in_auto_scaling",
      false,
      "Whether to consider off-diagonals when determining automatic scaling factors.");
  params.addRangeCheckedParam<Real>(
      "resid_vs_jac_scaling_param",
      0,
      "0<=resid_vs_jac_scaling_param<=1",
      "A parameter that indicates the weighting of the residual vs the Jacobian in determining "
      "variable scaling parameters. A value of 1 indicates pure residual-based scaling. A value of "
      "0 indicates pure Jacobian-based scaling");
  params.addParam<std::vector<std::vector<std::string>>>(
      "scaling_group_variables",
      "Name of variables that are grouped together for determining scale factors. (Multiple "
      "groups can be provided, separated by semicolon)");
  params.addParam<std::vector<std::string>>(
      "ignore_variables_for_autoscaling",
      "List of variables that do not participate in autoscaling.");
  params.addRangeCheckedParam<unsigned int>(
      "num_grids",
      1,
      "num_grids>0",
      "The number of grids to use for a grid sequencing algorithm. This includes the final grid, "
      "so num_grids = 1 indicates just one solve in a time-step");
  params.addParam<bool>("residual_and_jacobian_together",
                        false,
                        "Whether to compute the residual and Jacobian together.");

  params.addParam<bool>("reuse_preconditioner",
                        false,
                        "If true reuse the previously calculated "
                        "preconditioner for the linearized "
                        "system across multiple solves "
                        "spanning nonlinear iterations and time steps. "
                        "The preconditioner resets as controlled by "
                        "reuse_preconditioner_max_linear_its");
  params.addParam<unsigned int>("reuse_preconditioner_max_linear_its",
                                25,
                                "Reuse the previously calculated "
                                "preconditioner for the linear system "
                                "until the number of linear iterations "
                                "exceeds this number");

  params.addParamNamesToGroup("l_tol l_abs_tol l_max_its reuse_preconditioner "
                              "reuse_preconditioner_max_linear_its",
                              "Linear Solver");
  params.addParamNamesToGroup("solve_type nl_max_its nl_forced_its nl_max_funcs "
                              "nl_abs_tol nl_rel_tol nl_abs_step_tol nl_rel_step_tol "
                              "snesmf_reuse_base compute_initial_residual_before_preset_bcs "
                              "num_grids nl_div_tol nl_abs_div_tol residual_and_jacobian_together "
                              "n_max_nonlinear_pingpong",
                              "Nonlinear Solver");
  params.addParamNamesToGroup(
      "automatic_scaling compute_scaling_once off_diagonals_in_auto_scaling "
      "scaling_group_variables resid_vs_jac_scaling_param ignore_variables_for_autoscaling",
      "Solver variable scaling");
  params.addParamNamesToGroup("line_search line_search_package contact_line_search_ltol "
                              "contact_line_search_allowed_lambda_cuts",
                              "Solver line search");
  params.addParamNamesToGroup("skip_exception_check", "Advanced");

  return params;
}

FEProblemSolve::FEProblemSolve(Executioner & ex)
  : SolveObject(ex),
    _splitting(getParam<std::vector<std::string>>("splitting")),
    _num_grid_steps(getParam<unsigned int>("num_grids") - 1)
{
  if (_moose_line_searches.find(getParam<MooseEnum>("line_search").operator std::string()) !=
      _moose_line_searches.end())
    _problem.addLineSearch(_pars);

  // Extract and store PETSc related settings on FEProblemBase
  Moose::PetscSupport::storePetscOptions(_problem, _pars);

  EquationSystems & es = _problem.es();
  es.parameters.set<Real>("linear solver tolerance") = getParam<Real>("l_tol");

  es.parameters.set<Real>("linear solver absolute tolerance") = getParam<Real>("l_abs_tol");

  es.parameters.set<unsigned int>("linear solver maximum iterations") =
      getParam<unsigned int>("l_max_its");

  es.parameters.set<unsigned int>("nonlinear solver maximum iterations") =
      getParam<unsigned int>("nl_max_its");

  es.parameters.set<unsigned int>("nonlinear solver maximum function evaluations") =
      getParam<unsigned int>("nl_max_funcs");

  es.parameters.set<Real>("nonlinear solver absolute residual tolerance") =
      getParam<Real>("nl_abs_tol");

  es.parameters.set<Real>("nonlinear solver relative residual tolerance") =
      getParam<Real>("nl_rel_tol");

  es.parameters.set<Real>("nonlinear solver divergence tolerance") = getParam<Real>("nl_div_tol");

  es.parameters.set<Real>("nonlinear solver absolute step tolerance") =
      getParam<Real>("nl_abs_step_tol");

  es.parameters.set<Real>("nonlinear solver relative step tolerance") =
      getParam<Real>("nl_rel_step_tol");

  es.parameters.set<bool>("reuse preconditioner") = getParam<bool>("reuse_preconditioner");

  es.parameters.set<unsigned int>("reuse preconditioner maximum linear iterations") =
      getParam<unsigned int>("reuse_preconditioner_max_linear_its");

  _nl._compute_initial_residual_before_preset_bcs =
      getParam<bool>("compute_initial_residual_before_preset_bcs");

  _problem.setSNESMFReuseBase(getParam<bool>("snesmf_reuse_base"),
                              _pars.isParamSetByUser("snesmf_reuse_base"));

  _problem.skipExceptionCheck(getParam<bool>("skip_exception_check"));

  _problem.setMaxNLPingPong(getParam<unsigned int>("n_max_nonlinear_pingpong"));

  _problem.setNonlinearForcedIterations(getParam<unsigned int>("nl_forced_its"));

  _problem.setNonlinearAbsoluteDivergenceTolerance(getParam<Real>("nl_abs_div_tol"));

  _nl.setDecomposition(_splitting);

  if (getParam<bool>("residual_and_jacobian_together"))
    _nl.residualAndJacobianTogether();

  // Check whether the user has explicitly requested automatic scaling and is using a solve type
  // without a matrix. If so, then we warn them
  if ((_pars.isParamSetByUser("automatic_scaling") && getParam<bool>("automatic_scaling")) &&
      _problem.solverParams()._type == Moose::ST_JFNK)
  {
    paramWarning("automatic_scaling",
                 "Automatic scaling isn't implemented for the case where you do not have a "
                 "preconditioning matrix. No scaling will be applied");
    _problem.automaticScaling(false);
  }
  else
    // Check to see whether automatic_scaling has been specified anywhere, including at the
    // application level. No matter what: if we don't have a matrix, we don't do scaling
    _problem.automaticScaling((isParamValid("automatic_scaling")
                                   ? getParam<bool>("automatic_scaling")
                                   : getMooseApp().defaultAutomaticScaling()) &&
                              (_problem.solverParams()._type != Moose::ST_JFNK));

  _nl.computeScalingOnce(getParam<bool>("compute_scaling_once"));
  _nl.autoScalingParam(getParam<Real>("resid_vs_jac_scaling_param"));
  _nl.offDiagonalsInAutoScaling(getParam<bool>("off_diagonals_in_auto_scaling"));
  if (isParamValid("scaling_group_variables"))
    _nl.scalingGroupVariables(
        getParam<std::vector<std::vector<std::string>>>("scaling_group_variables"));
  if (isParamValid("ignore_variables_for_autoscaling"))
  {
    // Before setting ignore_variables_for_autoscaling, check that they are not present in
    // scaling_group_variables
    if (isParamValid("scaling_group_variables"))
    {
      const auto & ignore_variables_for_autoscaling =
          getParam<std::vector<std::string>>("ignore_variables_for_autoscaling");
      const auto & scaling_group_variables =
          getParam<std::vector<std::vector<std::string>>>("scaling_group_variables");
      for (const auto & group : scaling_group_variables)
        for (const auto & var_name : group)
          if (std::find(ignore_variables_for_autoscaling.begin(),
                        ignore_variables_for_autoscaling.end(),
                        var_name) != ignore_variables_for_autoscaling.end())
            paramError("ignore_variables_for_autoscaling",
                       "Variables cannot be in a scaling grouping and also be ignored");
    }
    _nl.ignoreVariablesForAutoscaling(
        getParam<std::vector<std::string>>("ignore_variables_for_autoscaling"));
  }

  _problem.numGridSteps(_num_grid_steps);
}

bool
FEProblemSolve::solve()
{
  // This loop is for nonlinear multigrids (developed by Alex)
  for (MooseIndex(_num_grid_steps) grid_step = 0; grid_step <= _num_grid_steps; ++grid_step)
  {
    _problem.solve();

    if (_problem.shouldSolve())
    {
      if (_problem.converged())
        _console << COLOR_GREEN << " Solve Converged!" << COLOR_DEFAULT << std::endl;
      else
      {
        _console << COLOR_RED << " Solve Did NOT Converge!" << COLOR_DEFAULT << std::endl;
        return false;
      }
    }
    else
      _console << COLOR_GREEN << " Solve Skipped!" << COLOR_DEFAULT << std::endl;

    if (grid_step != _num_grid_steps)
      _problem.uniformRefine();
  }
  return _problem.converged();
}
