//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SingleParameterInverseSolveAction.h"

#include "FEProblemBase.h"
#include "Factory.h"
#include "Control.h"
#include "MooseUtils.h"

registerMooseAction("OptimizationApp", SingleParameterInverseSolveAction, "add_multi_app");
registerMooseAction("OptimizationApp", SingleParameterInverseSolveAction, "add_transfer");
registerMooseAction("OptimizationApp", SingleParameterInverseSolveAction, "add_postprocessor");
registerMooseAction("OptimizationApp", SingleParameterInverseSolveAction, "add_convergence");
registerMooseAction("OptimizationApp", SingleParameterInverseSolveAction, "add_control");

InputParameters
SingleParameterInverseSolveAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Generates the full fixed-point inverse-solve workflow (forward sub-app, transfers, "
      "postprocessors, convergence, and a secant or Newton inversion control) from a single "
      "block.");

  MooseEnum method("secant newton", "secant");
  params.addParam<MooseEnum>("method", method, "Inversion update method.");
  params.addRequiredParam<FileName>("forward_input", "Forward-model sub-app input file.");
  params.addRequiredParam<PostprocessorName>(
      "sub_parameter_postprocessor", "Sub-app postprocessor that receives the parameter value.");
  params.addRequiredParam<PostprocessorName>(
      "sub_output_postprocessor", "Sub-app postprocessor holding the output to match the target.");
  params.addRequiredParam<FunctionName>("target_function", "Target output f(t).");
  params.addParam<Real>("initial_parameter", 1.0, "Initial guess for the parameter.");
  params.addRangeCheckedParam<Real>(
      "perturbation",
      1e-3,
      "perturbation>0",
      "Perturbation used to seed/compute the derivative (initial_delta for secant, "
      "parameter_delta for newton).");
  params.addRangeCheckedParam<Real>("absolute_tolerance",
                                    1e-8,
                                    "absolute_tolerance>0",
                                    "Absolute tolerance on |output - target|.");
  params.addRangeCheckedParam<Real>(
      "relative_tolerance",
      1e-6,
      "relative_tolerance>0",
      "Relative tolerance on |output - target|, relative to |target|.");
  params.addParam<unsigned int>(
      "max_iterations",
      50,
      "Maximum number of fixed-point iterations on the generated convergence.");
  params.addParam<bool>(
      "accept_on_max_iterations",
      false,
      "If true, accept the current estimate when the fixed-point loop reaches max_iterations "
      "instead of diverging, cutting the time step, and erroring.");
  params.addParam<PostprocessorName>(
      "result_postprocessor",
      "inverse_parameter",
      "Name of the created postprocessor holding the converged parameter (output to CSV).");

  return params;
}

SingleParameterInverseSolveAction::SingleParameterInverseSolveAction(
    const InputParameters & parameters)
  : Action(parameters)
{
}

void
SingleParameterInverseSolveAction::act()
{
  // Derive the generated-object name prefix in snake_case (e.g. single_parameter_inverse_solve).
  const std::string p = MooseUtils::camelCaseToUnderscore(name());
  const bool newton = getParam<MooseEnum>("method") == "newton";
  const std::string residual = p + "_residual";
  const PostprocessorName result = getParam<PostprocessorName>("result_postprocessor");

  if (_current_task == "add_multi_app")
  {
    auto ps = _factory.getValidParams("TransientMultiApp");
    ps.set<std::vector<FileName>>("input_files") = {getParam<FileName>("forward_input")};
    ps.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_BEGIN};
    _problem->addMultiApp("TransientMultiApp", p + "_forward", ps);
  }
  else if (_current_task == "add_transfer")
  {
    auto t1 = _factory.getValidParams("MultiAppPostprocessorTransfer");
    t1.set<MultiAppName>("to_multi_app") = p + "_forward";
    t1.set<PostprocessorName>("from_postprocessor") = p + "_param";
    t1.set<PostprocessorName>("to_postprocessor") =
        getParam<PostprocessorName>("sub_parameter_postprocessor");
    _problem->addTransfer("MultiAppPostprocessorTransfer", p + "_to_forward", t1);

    auto t2 = _factory.getValidParams("MultiAppPostprocessorTransfer");
    t2.set<MultiAppName>("from_multi_app") = p + "_forward";
    t2.set<PostprocessorName>("from_postprocessor") =
        getParam<PostprocessorName>("sub_output_postprocessor");
    t2.set<PostprocessorName>("to_postprocessor") = p + "_output";
    t2.set<MooseEnum>("reduction_type") = "average";
    _problem->addTransfer("MultiAppPostprocessorTransfer", p + "_from_forward", t2);
  }
  else if (_current_task == "add_postprocessor")
  {
    const std::vector<OutputName> none = {"none"};

    // Working parameter guess (transferred to the sub-app, read and written by the control).
    auto pp = _factory.getValidParams("Receiver");
    pp.set<Real>("default") = getParam<Real>("initial_parameter");
    pp.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
    pp.set<std::vector<OutputName>>("outputs") = none;
    _problem->addPostprocessor("Receiver", p + "_param", pp);

    // Sub-app output (filled by the FROM transfer).
    auto po = _factory.getValidParams("Receiver");
    po.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
    po.set<std::vector<OutputName>>("outputs") = none;
    _problem->addPostprocessor("Receiver", p + "_output", po);

    // Published converged parameter (the CSV output of interest).
    auto pr = _factory.getValidParams("Receiver");
    pr.set<Real>("default") = 0.0;
    _problem->addPostprocessor("Receiver", result, pr);

    // Convergence residual, written by the control each iteration (normalized so convergence is
    // declared at a fixed tolerance of 1). Large default so a step is never "converged" pre-solve.
    auto pres = _factory.getValidParams("Receiver");
    pres.set<Real>("default") = 1e30;
    pres.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
    pres.set<std::vector<OutputName>>("outputs") = none;
    _problem->addPostprocessor("Receiver", residual, pres);
  }
  else if (_current_task == "add_convergence")
  {
    auto pc = _factory.getValidParams("PostprocessorConvergence");
    pc.set<PostprocessorName>("postprocessor") = residual;
    pc.set<Real>("tolerance") = 1.0;
    pc.set<unsigned int>("max_iterations") = getParam<unsigned int>("max_iterations");
    pc.set<bool>("converge_at_max_iterations") = getParam<bool>("accept_on_max_iterations");
    _problem->addConvergence("PostprocessorConvergence", p + "_convergence", pc);
  }
  else if (_current_task == "add_control")
  {
    // The action cannot enable the fixed-point loop itself (that is frozen when the executioner is
    // constructed, before actions run), but by this task the executioner's convergence choice is
    // finalized, so verify the user pointed it at this action's convergence. Otherwise the loop is
    // disabled or driven by a different convergence and the inverse solve silently would not run.
    const std::string conv_name = p + "_convergence";
    if (!_problem->hasSetMultiAppFixedPointConvergenceName() ||
        _problem->getMultiAppFixedPointConvergenceName() != conv_name)
      mooseError("[",
                 p,
                 "] requires the executioner to use its generated convergence to enable and drive "
                 "the fixed-point loop. Add\n\n    multiapp_fixed_point_convergence = ",
                 conv_name,
                 "\n\nto the [Executioner] block. It is currently ",
                 _problem->hasSetMultiAppFixedPointConvergenceName()
                     ? "set to '" + _problem->getMultiAppFixedPointConvergenceName() + "'"
                     : "not set",
                 ".");

    const std::string ctype = newton ? "NewtonInversionControl" : "SecantInversionControl";
    auto pctl = _factory.getValidParams(ctype);
    pctl.set<PostprocessorName>("output_postprocessor") = p + "_output";
    pctl.set<PostprocessorName>("parameter_postprocessor") = p + "_param";
    pctl.set<PostprocessorName>("converged_parameter_postprocessor") = result;
    pctl.set<PostprocessorName>("residual_postprocessor") = residual;
    pctl.set<FunctionName>("target_function") = getParam<FunctionName>("target_function");
    pctl.set<Real>("absolute_tolerance") = getParam<Real>("absolute_tolerance");
    pctl.set<Real>("relative_tolerance") = getParam<Real>("relative_tolerance");
    if (newton)
      pctl.set<Real>("parameter_delta") = getParam<Real>("perturbation");
    else
      pctl.set<Real>("initial_delta") = getParam<Real>("perturbation");

    pctl.addPrivateParam<FEProblemBase *>("_fe_problem_base", _problem.get());
    auto control = _factory.create<Control>(ctype, p + "_control", pctl);
    _problem->getControlWarehouse().addObject(control);
  }
}
