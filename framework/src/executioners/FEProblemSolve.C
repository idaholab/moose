//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblemSolve.h"

#include "FEProblem.h"
#include "NonlinearSystemBase.h"
#include "Convergence.h"
#include "Executioner.h"
#include "ConvergenceIterationTypes.h"
#include "MooseUtils.h"
#include "Executor.h"
#include "Factory.h"

std::set<std::string> const FEProblemSolve::_moose_line_searches = {"contact", "project"};

const std::set<std::string> &
FEProblemSolve::mooseLineSearches()
{
  return _moose_line_searches;
}

InputParameters
FEProblemSolve::mooseLineSearchParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<unsigned>("contact_line_search_allowed_lambda_cuts",
                            2,
                            "The number of times lambda is allowed to be cut in half in the "
                            "contact line search. We recommend this number be roughly bounded by 0 "
                            "<= allowed_lambda_cuts <= 3");
  params.addParam<Real>("contact_line_search_ltol",
                        "The linear relative tolerance to be used while the contact state is "
                        "changing between non-linear iterations. We recommend that this tolerance "
                        "be looser than the standard linear tolerance");
  params.addParamNamesToGroup("contact_line_search_ltol contact_line_search_allowed_lambda_cuts",
                              "Solver line search");
  return params;
}

InputParameters
FEProblemSolve::feProblemDefaultConvergenceParams()
{
  InputParameters params = emptyInputParameters();

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
  params.addParam<unsigned int>("n_max_nonlinear_pingpong",
                                100,
                                "The maximum number of times the nonlinear residual can ping pong "
                                "before requesting halting the current evaluation and requesting "
                                "timestep cut for transient simulations");

  params.addParamNamesToGroup(
      "nl_max_its nl_forced_its nl_max_funcs nl_abs_tol nl_rel_tol "
      "nl_rel_step_tol nl_abs_step_tol nl_div_tol nl_abs_div_tol n_max_nonlinear_pingpong",
      "Nonlinear Solver");

  return params;
}

InputParameters
FEProblemSolve::validParams()
{
  InputParameters params = MultiSystemSolveObject::validParams();
  params += FEProblemSolve::feProblemDefaultConvergenceParams();

  params += FEProblemSolve::mooseLineSearchParams();

  params += Moose::PetscSupport::flagAndPairOptions();
  params += Moose::PetscSupport::kspRelatedParams();
  params += Moose::PetscSupport::newtonKrylovParams();
  params.addParam<std::vector<ConvergenceName>>(
      "nonlinear_convergence",
      "Name of the Convergence object(s) to use to assess convergence of the "
      "nonlinear system(s) solve. If not provided, the default Convergence "
      "associated with the Problem will be constructed internally.");
  params.addParam<std::vector<ConvergenceName>>(
      "linear_convergence",
      "Name of the Convergence object(s) to use to assess convergence of the "
      "linear system(s) solve. If not provided, the linear solver tolerance parameters are used");
  params.addParam<bool>(
      "snesmf_reuse_base",
      true,
      "Specifies whether or not to reuse the base vector for matrix-free calculation");
  params.addParam<bool>(
      "skip_exception_check", false, "Specifies whether or not to skip exception check");
  params.addParam<bool>(
      "use_pre_SMO_residual",
      false,
      "Compute the pre-SMO residual norm and use it in the relative convergence check. The "
      "pre-SMO residual is computed at the begining of the time step before solution-modifying "
      "objects are executed. Solution-modifying objects include preset BCs, constraints, "
      "predictors, etc.");
  params.addParam<bool>("automatic_scaling", "Whether to use automatic scaling for the variables.");
  params.addParam<std::vector<bool>>(
      "compute_scaling_once",
      {true},
      "Whether the scaling factors should only be computed once at the beginning of the simulation "
      "through an extra Jacobian evaluation. If this is set to false, then the scaling factors "
      "will be computed during an extra Jacobian evaluation at the beginning of every time step. "
      "Vector entries correspond to each nonlinear system.");
  params.addParam<std::vector<bool>>(
      "off_diagonals_in_auto_scaling",
      {false},
      "Whether to consider off-diagonals when determining automatic scaling factors. Vector "
      "entries correspond to each nonlinear system.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "resid_vs_jac_scaling_param",
      {0},
      "0<=resid_vs_jac_scaling_param<=1",
      "A parameter that indicates the weighting of the residual vs the Jacobian in determining "
      "variable scaling parameters. A value of 1 indicates pure residual-based scaling. A value of "
      "0 indicates pure Jacobian-based scaling. Vector entries correspond to each nonlinear "
      "system.");
  params.addParam<std::vector<std::vector<std::vector<std::string>>>>(
      "scaling_group_variables",
      "Name of variables that are grouped together for determining scale factors. (Multiple "
      "groups can be provided, separated by semicolon). Vector entries correspond to each "
      "nonlinear system.");
  params.addParam<std::vector<std::vector<std::string>>>(
      "ignore_variables_for_autoscaling",
      "List of variables that do not participate in autoscaling. Vector entries correspond to each "
      "nonlinear system.");
  params.addRangeCheckedParam<unsigned int>(
      "num_grids",
      1,
      "num_grids>0",
      "The number of grids to use for a grid sequencing algorithm. This includes the final grid, "
      "so num_grids = 1 indicates just one solve in a time-step");
  params.addParam<std::vector<bool>>("residual_and_jacobian_together",
                                     {false},
                                     "Whether to compute the residual and Jacobian together. "
                                     "Vector entries correspond to each nonlinear system.");

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
  params.addParamNamesToGroup(
      "solve_type snesmf_reuse_base use_pre_SMO_residual "
      "num_grids residual_and_jacobian_together nonlinear_convergence linear_convergence",
      "Nonlinear Solver");
  params.addParamNamesToGroup(
      "automatic_scaling compute_scaling_once off_diagonals_in_auto_scaling "
      "scaling_group_variables resid_vs_jac_scaling_param ignore_variables_for_autoscaling",
      "Solver variable scaling");
  params.addParamNamesToGroup("skip_exception_check", "Advanced");

  return params;
}

FEProblemSolve::FEProblemSolve(Executioner & ex)
  : MultiSystemSolveObject(ex),
    _num_grid_steps(cast_int<unsigned int>(getParam<unsigned int>("num_grids") - 1))
{
  if (_moose_line_searches.find(getParam<MooseEnum>("line_search").operator std::string()) !=
      _moose_line_searches.end())
    _problem.addLineSearch(_pars);

  // Set linear solve parameters in the equation system
  // Nonlinear solve parameters are added in the DefaultNonlinearConvergence
  EquationSystems & es = _problem.es();
  Moose::PetscSupport::setESLinearSolverParams(es, *this);
  es.parameters.set<bool>("reuse preconditioner") = getParam<bool>("reuse_preconditioner");
  es.parameters.set<unsigned int>("reuse preconditioner maximum linear iterations") =
      getParam<unsigned int>("reuse_preconditioner_max_linear_its");

  // Transfer to the Problem misc nonlinear solve optimization parameters
  _problem.setSNESMFReuseBase(getParam<bool>("snesmf_reuse_base"),
                              _pars.isParamSetByUser("snesmf_reuse_base"));
  _problem.skipExceptionCheck(getParam<bool>("skip_exception_check"));

  if (isParamValid("nonlinear_convergence"))
  {
    if (_problem.onlyAllowDefaultNonlinearConvergence())
      mooseError("The selected problem does not allow 'nonlinear_convergence' to be set.");
    _problem.setNonlinearConvergenceNames(
        getParam<std::vector<ConvergenceName>>("nonlinear_convergence"));
  }
  else
    _problem.setNeedToAddDefaultNonlinearConvergence();
  if (isParamValid("linear_convergence"))
  {
    if (_problem.numLinearSystems() == 0)
      paramError(
          "linear_convergence",
          "Setting 'linear_convergence' is currently only possible for solving linear systems");
    _problem.setLinearConvergenceNames(
        getParam<std::vector<ConvergenceName>>("linear_convergence"));
  }

  // The executors store the PETSc options and the solve types, which the automatic scaling checks
  // below read. They must come after the line search and the convergence names.
  addExecutors();

  // Check whether the user has explicitly requested automatic scaling and is using a solve type
  // without a matrix. If so, then we warn them
  if ((_pars.isParamSetByUser("automatic_scaling") && getParam<bool>("automatic_scaling")) &&
      std::all_of(_systems.begin(),
                  _systems.end(),
                  [this](const auto & solver_sys)
                  { return _problem.solverParams(solver_sys->number())._type == Moose::ST_JFNK; }))
  {
    paramWarning("automatic_scaling",
                 "Automatic scaling isn't implemented for the case where you do not have a "
                 "preconditioning matrix. No scaling will be applied");
    _problem.automaticScaling(false);
  }
  else
    // Check to see whether automatic_scaling has been specified anywhere, including at the
    // application level. No matter what: if we don't have a matrix, we don't do scaling
    _problem.automaticScaling(
        isParamValid("automatic_scaling")
            ? getParam<bool>("automatic_scaling")
            : (getMooseApp().defaultAutomaticScaling() &&
               std::any_of(_systems.begin(),
                           _systems.end(),
                           [this](const auto & solver_sys)
                           {
                             return _problem.solverParams(solver_sys->number())._type !=
                                    Moose::ST_JFNK;
                           })));

  if (!_using_multi_sys_fp_iterations && isParamValid("multi_system_fixed_point_convergence"))
    paramError("multi_system_fixed_point_convergence",
               "Cannot set a convergence object for multi-system fixed point iterations if "
               "'multi_system_fixed_point' is set to false");
  if (_using_multi_sys_fp_iterations && !isParamValid("multi_system_fixed_point_convergence"))
    paramError("multi_system_fixed_point_convergence",
               "Must set a convergence object for multi-system fixed point iterations if using "
               "multi-system fixed point iterations");

  // Set the same parameters to every nonlinear system by default
  int i_nl_sys = -1;
  for (const auto i_sys : index_range(_systems))
  {
    auto nl_ptr = dynamic_cast<NonlinearSystemBase *>(_systems[i_sys]);
    // Linear systems have very different parameters at the moment
    if (!nl_ptr)
      continue;
    auto & nl = *nl_ptr;
    i_nl_sys++;

    nl.setPreSMOResidual(getParam<bool>("use_pre_SMO_residual"));

    const auto res_and_jac =
        getParamFromNonlinearSystemVectorParam<bool>("residual_and_jacobian_together", i_nl_sys);
    if (res_and_jac)
      nl.residualAndJacobianTogether();

    // Automatic scaling parameters
    nl.computeScalingOnce(
        getParamFromNonlinearSystemVectorParam<bool>("compute_scaling_once", i_nl_sys));
    nl.autoScalingParam(
        getParamFromNonlinearSystemVectorParam<Real>("resid_vs_jac_scaling_param", i_nl_sys));
    nl.offDiagonalsInAutoScaling(
        getParamFromNonlinearSystemVectorParam<bool>("off_diagonals_in_auto_scaling", i_nl_sys));
    if (isParamValid("scaling_group_variables"))
      nl.scalingGroupVariables(
          getParamFromNonlinearSystemVectorParam<std::vector<std::vector<std::string>>>(
              "scaling_group_variables", i_nl_sys));
    if (isParamValid("ignore_variables_for_autoscaling"))
    {
      // Before setting ignore_variables_for_autoscaling, check that they are not present in
      // scaling_group_variables
      if (isParamValid("scaling_group_variables"))
      {
        const auto & ignore_variables_for_autoscaling =
            getParamFromNonlinearSystemVectorParam<std::vector<std::string>>(
                "ignore_variables_for_autoscaling", i_nl_sys);
        const auto & scaling_group_variables =
            getParamFromNonlinearSystemVectorParam<std::vector<std::vector<std::string>>>(
                "scaling_group_variables", i_nl_sys);
        for (const auto & group : scaling_group_variables)
          for (const auto & var_name : group)
            if (std::find(ignore_variables_for_autoscaling.begin(),
                          ignore_variables_for_autoscaling.end(),
                          var_name) != ignore_variables_for_autoscaling.end())
              paramError("ignore_variables_for_autoscaling",
                         "Variables cannot be in a scaling grouping and also be ignored");
      }
      nl.ignoreVariablesForAutoscaling(
          getParamFromNonlinearSystemVectorParam<std::vector<std::string>>(
              "ignore_variables_for_autoscaling", i_nl_sys));
    }
  }

  // Multi-grid options
  _problem.numGridSteps(_num_grid_steps);
}

template <typename T>
T
FEProblemSolve::getParamFromNonlinearSystemVectorParam(const std::string & param_name,
                                                       unsigned int index) const
{
  const auto & param_vec = getParam<std::vector<T>>(param_name);
  if (index > _num_nl_systems)
    paramError(param_name,
               "Vector parameter is requested at index (" + std::to_string(index) +
                   ") which is larger than number of nonlinear systems (" +
                   std::to_string(_num_nl_systems) + ").");
  if (param_vec.size() == 0)
    paramError(
        param_name,
        "This parameter was passed to a routine which cannot handle empty vector parameters");
  if (param_vec.size() != 1 && param_vec.size() != _num_nl_systems)
    paramError(param_name,
               "Vector parameter size (" + std::to_string(param_vec.size()) +
                   ") is different than the number of nonlinear systems (" +
                   std::to_string(_num_nl_systems) + ").");

  // User passed only one parameter, assume it applies to all nonlinear systems
  if (param_vec.size() == 1)
    return param_vec[0];
  else
    return param_vec[index];
}

void
FEProblemSolve::addExecutors()
{
  auto & factory = _app.getFactory();
  // The executors run inside this object, so they register no execute flags and no test-restep
  // notice of their own
  const auto set_internal = [](InputParameters & params)
  {
    params.set<ExecFlagType>("begin_exec_flag") = EXEC_NONE;
    params.set<ExecFlagType>("end_exec_flag") = EXEC_NONE;
    params.set<bool>("_supports_test_restep") = true;
  };

  // The executor parameters that come from this object. The Newton-Krylov parameters apply to
  // nonlinear systems only.
  auto forwarded = Moose::PetscSupport::flagAndPairOptions().getParametersList();
  std::vector<std::string> linear_forwarded(forwarded.begin(), forwarded.end());
  forwarded.merge(Moose::PetscSupport::newtonKrylovParams().getParametersList());
  const std::vector<std::string> nonlinear_forwarded(forwarded.begin(), forwarded.end());

  std::vector<ExecutorName> system_executors;
  for (const auto i : index_range(_systems))
  {
    const auto & sys = *_systems[i];
    auto params = factory.getValidParams("SystemSolveExecutor");
    params.applySpecificParameters(
        _pars,
        dynamic_cast<const NonlinearSystemBase *>(&sys) ? nonlinear_forwarded : linear_forwarded);
    set_internal(params);
    params.set<SolverSystemName>("system") = sys.name();
    params.set<Real>("relaxation_factor") = _multi_sys_fp_relax_factors[i];
    params.set<bool>("_print_system_name") = _systems.size() > 1;
    // This object sets the linear solver tolerances on the EquationSystems
    params.set<bool>("_problem_wide_linear_tolerances") = true;

    system_executors.push_back("_moose_solve_" + sys.name());
    _app.addExecutor("SystemSolveExecutor", system_executors.back(), params);
    _executors.push_back(&_app.getExecutor<Executor>(system_executors.back()));
  }

  auto params = factory.getValidParams("FixedPointExecutor");
  set_internal(params);
  params.set<std::vector<ExecutorName>>("inner_executors") = system_executors;
  if (_using_multi_sys_fp_iterations && isParamValid("multi_system_fixed_point_convergence"))
    params.set<ConvergenceName>("convergence") =
        getParam<ConvergenceName>("multi_system_fixed_point_convergence");
  const ExecutorName fixed_point_name = "_moose_fixed_point";
  _app.addExecutor("FixedPointExecutor", fixed_point_name, params);
  _fixed_point_executor = &_app.getExecutor<Executor>(fixed_point_name);
  _executors.push_back(_fixed_point_executor);
}

void
FEProblemSolve::initialSetup()
{
  MultiSystemSolveObject::initialSetup();
  convergenceSetup();
  // Keep track of the solution warnings from the setup
  // before a count reset at the beginning of the time step
  if (!_app.isRecovering())
  {
    _app.solutionInvalidity().syncIteration();
    _app.solutionInvalidity().accumulateIterationIntoTimeStepOccurences();
    _app.solutionInvalidity().accumulateTimeStepIntoTotalOccurences(0);
  }

  // Convergence objects exist only after the executors
  for (auto * const executor : _executors)
    executor->initialSetup();
}

void
FEProblemSolve::convergenceSetup()
{
  // nonlinear
  const auto conv_names = _problem.getNonlinearConvergenceNames();
  for (const auto & conv_name : conv_names)
  {
    auto & conv = _problem.getConvergence(conv_name);
    conv.checkIterationType(ConvergenceIterationTypes::NONLINEAR);
  }

  // linear
  if (isParamValid("linear_convergence"))
  {
    const auto conv_names = getParam<std::vector<ConvergenceName>>("linear_convergence");
    for (const auto & conv_name : conv_names)
    {
      auto & conv = _problem.getConvergence(conv_name);
      conv.checkIterationType(ConvergenceIterationTypes::LINEAR);
    }
  }
}

bool
FEProblemSolve::solve()
{
  // Outer loop for multi-grid convergence
  bool converged = false;
  for (MooseIndex(_num_grid_steps) grid_step = 0; grid_step <= _num_grid_steps; ++grid_step)
  {
    const auto result = _fixed_point_executor->exec();
    // A failed system solve ends the solve. A fixed point iteration that does not converge ends
    // only this grid step.
    if (result.converged && !result.convergedAll())
      return false;
    converged = result.converged;

    if (grid_step != _num_grid_steps)
      _problem.uniformRefine();
  }

  return converged;
}
