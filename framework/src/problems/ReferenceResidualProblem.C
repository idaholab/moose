//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ReferenceResidualProblem.h"

#include "AuxiliarySystem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "libmesh/enum_norm_type.h"
#include "libmesh/utility.h"

registerMooseObject("MooseApp", ReferenceResidualProblem);

InputParameters
ReferenceResidualProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addClassDescription("Problem that checks for convergence relative to "
                             "a user-supplied reference quantity rather than "
                             "the initial residual");
  params.addParam<std::vector<NonlinearVariableName>>(
      "solution_variables", "Set of solution variables to be checked for relative convergence");
  params.addParam<std::vector<AuxVariableName>>(
      "reference_residual_variables",
      "Set of variables that provide reference residuals for relative convergence check");
  params.addParam<TagName>("reference_vector", "The tag name of the reference residual vector.");
  params.addParam<Real>("acceptable_multiplier",
                        1.0,
                        "Multiplier applied to relative tolerance for acceptable limit");
  params.addParam<int>("acceptable_iterations",
                       0,
                       "Iterations after which convergence to acceptable limits is accepted");
  params.addParam<std::vector<std::vector<NonlinearVariableName>>>(
      "group_variables",
      "Name of variables that are grouped together to check convergence. (Multiple groups can be "
      "provided, separated by semicolon)");
  params.addParam<std::vector<NonlinearVariableName>>(
      "converge_on",
      "If supplied, use only these variables in the individual variable convergence check");
  return params;
}

ReferenceResidualProblem::ReferenceResidualProblem(const InputParameters & params)
  : FEProblem(params),
    _use_group_variables(false),
    _reference_vector(nullptr),
    _converge_on(getParam<std::vector<NonlinearVariableName>>("converge_on"))
{
  // Create reference residual tag
  const TagName tagname = "reference_residual_tag";
  auto tag = addVectorTag(tagname);
  for (unsigned int nl_sys_num = 0; nl_sys_num < _num_nl_sys; ++nl_sys_num)
  {
    auto nl = &getNonlinearSystem(nl_sys_num);
    nl[nl_sys_num].addVector(tag, false, GHOSTED);
  }

  if (params.isParamValid("solution_variables"))
  {
    if (params.isParamValid("reference_vector"))
      mooseDeprecated("The `solution_variables` parameter is deprecated, has no effect when "
                      "the tagging system is used, and will be removed on January 1, 2020. "
                      "Please simply delete this parameter from your input file.");
    _soln_var_names = params.get<std::vector<NonlinearVariableName>>("solution_variables");
  }

  if (params.isParamValid("reference_residual_variables") &&
      params.isParamValid("reference_vector"))
    mooseError(
        "For `ReferenceResidualProblem` you can specify either the `reference_residual_variables` "
        "or `reference_vector` parameter, not both. `reference_residual_variables` is deprecated "
        "so we recommend using `reference_vector`");

  if (params.isParamValid("reference_residual_variables"))
  {
    mooseDeprecated(
        "The save-in method for composing reference residual quantities is deprecated "
        "and will be removed on January 1, 2020. Please use the tagging system instead; "
        "specifically, please assign a TagName to the `reference_vector` parameter");

    _ref_resid_var_names = params.get<std::vector<AuxVariableName>>("reference_residual_variables");

    if (_soln_var_names.size() != _ref_resid_var_names.size())
      mooseError("In ReferenceResidualProblem, size of solution_variables (",
                 _soln_var_names.size(),
                 ") != size of reference_residual_variables (",
                 _ref_resid_var_names.size(),
                 ")");
  }
  else if (params.isParamValid("reference_vector"))
  {
    if (numNonlinearSystems() > 1)
      paramError(
          "nl_sys_names",
          "reference residual problem does not currently support multiple nonlinear systems");
    _reference_vector =
        &getNonlinearSystemBase(0).getVector(getVectorTagID(getParam<TagName>("reference_vector")));
  }
  else
    mooseInfo("Neither the `reference_residual_variables` nor `reference_vector` parameter is "
              "specified for `ReferenceResidualProblem`, which means that no reference "
              "quantites are set. Because of this, the standard technique of comparing the "
              "norm of the full residual vector with its initial value will be used.");

  if (params.isParamValid("group_variables"))
  {
    _group_variables =
        params.get<std::vector<std::vector<NonlinearVariableName>>>("group_variables");
    _use_group_variables = true;
  }

  _accept_mult = params.get<Real>("acceptable_multiplier");
  _accept_iters = params.get<int>("acceptable_iterations");
}

void
ReferenceResidualProblem::initialSetup()
{
  NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
  AuxiliarySystem & aux_sys = getAuxiliarySystem();
  System & s = nonlinear_sys.system();
  auto & as = aux_sys.sys();

  if (_soln_var_names.empty())
  {
    // If the user provides reference_vector, that implies that they want the
    // individual variables compared against their reference quantities in the
    // tag vector. The code depends on having _soln_var_names populated,
    // so fill that out if they didn't specify solution_variables.
    if (_reference_vector)
      for (unsigned int var_num = 0; var_num < s.n_vars(); var_num++)
        _soln_var_names.push_back(s.variable_name(var_num));

    // If they didn't provide reference_vector, that implies that they
    // want to skip the individual variable comparison, so leave it alone.
  }
  else if (_soln_var_names.size() != s.n_vars())
    mooseError("In ReferenceResidualProblem, size of solution_variables (",
               _soln_var_names.size(),
               ") != number of variables in system (",
               s.n_vars(),
               ")");

  const auto n_soln_vars = _soln_var_names.size();
  _variable_group_num_index.resize(n_soln_vars);

  if (!_converge_on.empty())
  {
    _converge_on_var.assign(n_soln_vars, false);
    for (std::size_t i = 0; i < n_soln_vars; ++i)
      for (const auto & c : _converge_on)
        if (MooseUtils::globCompare(_soln_var_names[i], c))
        {
          _converge_on_var[i] = true;
          break;
        }
  }
  else
    _converge_on_var.assign(n_soln_vars, true);

  unsigned int group_variable_num = 0;
  if (_use_group_variables)
  {
    for (unsigned int i = 0; i < _group_variables.size(); ++i)
    {
      group_variable_num += _group_variables[i].size();
      if (_group_variables[i].size() == 1)
        mooseError(" In the 'group_variables' parameter, variable ",
                   _group_variables[i][0],
                   " is not grouped with other variables.");
    }

    unsigned int size = n_soln_vars - group_variable_num + _group_variables.size();
    _group_ref_resid.resize(size);
    _group_resid.resize(size);
    _group_output_resid.resize(size);
    _group_soln_var_names.resize(size);
    _group_ref_resid_var_names.resize(size);
  }
  else
  {
    _group_ref_resid.resize(n_soln_vars);
    _group_resid.resize(n_soln_vars);
    _group_output_resid.resize(n_soln_vars);
    _group_soln_var_names.resize(n_soln_vars);
    _group_ref_resid_var_names.resize(n_soln_vars);
  }

  std::set<std::string> check_duplicate;
  if (_use_group_variables)
  {
    for (unsigned int i = 0; i < _group_variables.size(); ++i)
      for (unsigned int j = 0; j < _group_variables[i].size(); ++j)
        check_duplicate.insert(_group_variables[i][j]);

    if (check_duplicate.size() != group_variable_num)
      mooseError(
          "A variable cannot be included in multiple groups in the 'group_variables' parameter.");
  }

  _soln_vars.clear();
  for (unsigned int i = 0; i < n_soln_vars; ++i)
  {
    bool foundMatch = false;
    for (unsigned int var_num = 0; var_num < s.n_vars(); var_num++)
      if (_soln_var_names[i] == s.variable_name(var_num))
      {
        _soln_vars.push_back(var_num);
        foundMatch = true;
        break;
      }

    if (!foundMatch)
      mooseError("Could not find solution variable '", _soln_var_names[i], "' in system");
  }

  if (!_reference_vector)
  {
    _ref_resid_vars.clear();
    for (unsigned int i = 0; i < _ref_resid_var_names.size(); ++i)
    {
      bool foundMatch = false;
      for (unsigned int var_num = 0; var_num < as.n_vars(); var_num++)
        if (_ref_resid_var_names[i] == as.variable_name(var_num))
        {
          _ref_resid_vars.push_back(var_num);
          foundMatch = true;
          break;
        }

      if (!foundMatch)
        mooseError("Could not find variable '", _ref_resid_var_names[i], "' in auxiliary system");
    }
  }

  unsigned int ungroup_index = 0;
  if (_use_group_variables)
    ungroup_index = _group_variables.size();

  for (unsigned int i = 0; i < _soln_vars.size(); ++i)
  {
    bool find_group = false;
    if (_use_group_variables)
    {
      for (unsigned int j = 0; j < _group_variables.size(); ++j)
        if (std::find(_group_variables[j].begin(),
                      _group_variables[j].end(),
                      s.variable_name(_soln_vars[i])) != _group_variables[j].end())
        {
          if (!_converge_on_var[i])
            paramError("converge_on",
                       "You added variable '",
                       _soln_var_names[i],
                       "' to a group but excluded it from the convergence check. This is not "
                       "permitted.");

          _variable_group_num_index[i] = j;
          find_group = true;
          break;
        }

      if (!find_group)
      {
        _variable_group_num_index[i] = ungroup_index;
        ungroup_index++;
      }
    }
    else
      _variable_group_num_index[i] = i;
  }

  if (_use_group_variables)
  {
    for (unsigned int i = 0; i < _group_variables.size(); ++i)
    {
      unsigned int num_scalar_vars = 0;
      unsigned int num_field_vars = 0;
      if (_group_variables[i].size() > 1)
      {
        for (unsigned int j = 0; j < _group_variables[i].size(); ++j)
          for (unsigned int var_num = 0; var_num < s.n_vars(); var_num++)
            if (_group_variables[i][j] == s.variable_name(var_num))
            {
              if (nonlinear_sys.isScalarVariable(_soln_vars[var_num]))
                ++num_scalar_vars;
              else
                ++num_field_vars;
              break;
            }
      }
      if (num_scalar_vars > 0 && num_field_vars > 0)
        mooseWarning("In the 'group_variables' parameter, standard variables and scalar variables "
                     "are grouped together in group ",
                     i);
    }
  }

  for (unsigned int i = 0; i < n_soln_vars; ++i)
  {
    if (_group_soln_var_names[_variable_group_num_index[i]].empty())
    {
      _group_soln_var_names[_variable_group_num_index[i]] = _soln_var_names[i];
      if (_use_group_variables && _variable_group_num_index[i] < _group_variables.size())
        _group_soln_var_names[_variable_group_num_index[i]] += " (grouped) ";
    }

    if (!_reference_vector && _group_ref_resid_var_names[_variable_group_num_index[i]].empty())
    {
      _group_ref_resid_var_names[_variable_group_num_index[i]] = _ref_resid_var_names[i];
      if (_use_group_variables && _variable_group_num_index[i] < _group_variables.size())
        _group_ref_resid_var_names[_variable_group_num_index[i]] += " (grouped) ";
    }
  }

  if (!_reference_vector)
  {
    const unsigned int size_solnVars = _soln_vars.size();
    _scaling_factors.resize(size_solnVars);
    for (unsigned int i = 0; i < size_solnVars; ++i)
      if (nonlinear_sys.isScalarVariable(_soln_vars[i]))
        _scaling_factors[i] = nonlinear_sys.getScalarVariable(0, _soln_vars[i]).scalingFactor();
      else
        _scaling_factors[i] = nonlinear_sys.getVariable(/*tid*/ 0, _soln_vars[i]).scalingFactor();
  }

  FEProblemBase::initialSetup();
}

void
ReferenceResidualProblem::updateReferenceResidual()
{
  NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
  AuxiliarySystem & aux_sys = getAuxiliarySystem();
  System & s = nonlinear_sys.system();
  auto & as = aux_sys.sys();

  for (unsigned int i = 0; i < _group_resid.size(); ++i)
  {
    _group_resid[i] = 0.0;
    _group_output_resid[i] = 0.0;
    _group_ref_resid[i] = 0.0;
  }

  for (unsigned int i = 0; i < _soln_vars.size(); ++i)
  {
    const auto resid =
        Utility::pow<2>(s.calculate_norm(nonlinear_sys.RHS(), _soln_vars[i], DISCRETE_L2));
    const auto group = _variable_group_num_index[i];
    if (_local_norm)
    {
      mooseAssert(nonlinear_sys.RHS().size() == (*_reference_vector).size(),
                  "Sizes of nonlinear RHS and reference vector should be the same.");
      mooseAssert((*_reference_vector).size(), "Reference vector must be provided.");
      auto div = nonlinear_sys.RHS().clone();
      *div /= *_reference_vector;
      // Do magic to prevent 0/0, throw error if not zero divided by 0 occurs
      resid = Utility::pow<2>(s.calculate_norm(*div, _soln_vars[i], DISCRETE_L2));
    }
    else
    {
      resid = Utility::pow<2>(s.calculate_norm(nonlinear_sys.RHS(), _soln_vars[i], DISCRETE_L2));
      if (_reference_vector)
      {
        const auto ref_resid = s.calculate_norm(*_reference_vector, _soln_vars[i], DISCRETE_L2);
        _group_ref_resid[group] += Utility::pow<2>(ref_resid);
      }
    }

    _group_resid[group] += _converge_on_var[i] ? resid : 0;
    _group_output_resid[group] += resid;
  }

  if (!_reference_vector)
  {
    for (unsigned int i = 0; i < _ref_resid_vars.size(); ++i)
    {
      const auto ref_resid =
          as.calculate_norm(*as.current_local_solution, _ref_resid_vars[i], DISCRETE_L2) *
          _scaling_factors[i];
      _group_ref_resid[_variable_group_num_index[i]] += Utility::pow<2>(ref_resid);
    }
  }

  for (unsigned int i = 0; i < _group_resid.size(); ++i)
  {
    _group_resid[i] = std::sqrt(_group_resid[i]);
    _group_output_resid[i] = std::sqrt(_group_output_resid[i]);
    _group_ref_resid[i] = std::sqrt(_group_ref_resid[i]);
  }
}

MooseNonlinearConvergenceReason
ReferenceResidualProblem::checkNonlinearConvergence(std::string & msg,
                                                    const PetscInt it,
                                                    const Real xnorm,
                                                    const Real snorm,
                                                    const Real fnorm,
                                                    const Real rtol,
                                                    const Real /*divtol*/,
                                                    const Real stol,
                                                    const Real abstol,
                                                    const PetscInt nfuncs,
                                                    const PetscInt max_funcs,
                                                    const Real initial_residual_before_preset_bcs,
                                                    const Real /*div_threshold*/)
{
  updateReferenceResidual();

  if (_group_soln_var_names.size() > 0)
  {
    _console << "   Solution, reference convergence variable norms:\n";
    unsigned int maxwsv = 0;
    unsigned int maxwrv = 0;
    for (unsigned int i = 0; i < _group_soln_var_names.size(); ++i)
    {
      if (_group_soln_var_names[i].size() > maxwsv)
        maxwsv = _group_soln_var_names[i].size();
      if (!_reference_vector && _group_ref_resid_var_names[i].size() > maxwrv)
        maxwrv = _group_ref_resid_var_names[i].size();
    }
    if (_reference_vector)
      // maxwrv is the width of maxwsv plus the length of "_ref" (e.g. 4)
      maxwrv = maxwsv + 4;

    for (unsigned int i = 0; i < _group_soln_var_names.size(); ++i)
    {
      auto ref_var_name =
          _reference_vector ? _group_soln_var_names[i] + "_ref" : _group_ref_resid_var_names[i];
      _console << "   " << std::setw(maxwsv + 2) << std::left << _group_soln_var_names[i] + ":";

      if (_group_output_resid[i] == _group_resid[i])
        _console << _group_output_resid[i];
      else
        _console << _group_resid[i] << " (" << _group_output_resid[i] << ')';
      _console << "  " << std::setw(maxwrv + 2) << ref_var_name + ":" << _group_ref_resid[i]
               << '\n';
    }

    _console << std::flush;
  }

  NonlinearSystemBase & system = getNonlinearSystemBase();
  MooseNonlinearConvergenceReason reason = MooseNonlinearConvergenceReason::ITERATING;
  std::stringstream oss;

  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MooseNonlinearConvergenceReason::DIVERGED_FNORM_NAN;
  }
  else if (it >= _nl_forced_its && fnorm < abstol)
  {
    oss << "Converged due to function norm " << fnorm << " < " << abstol << std::endl;
    reason = MooseNonlinearConvergenceReason::CONVERGED_FNORM_ABS;
  }
  else if (nfuncs >= max_funcs)
  {
    oss << "Exceeded maximum number of function evaluations: " << nfuncs << " > " << max_funcs
        << std::endl;
    reason = MooseNonlinearConvergenceReason::DIVERGED_FUNCTION_COUNT;
  }

  if (it >= _nl_forced_its && reason == MooseNonlinearConvergenceReason::ITERATING)
  {
    if (checkConvergenceIndividVars(fnorm, abstol, rtol, initial_residual_before_preset_bcs))
    {
      if (_group_resid.size() > 0)
        oss << "Converged due to function norm "
            << " < "
            << " (relative tolerance) or (absolute tolerance) for all solution variables"
            << std::endl;
      else
        oss << "Converged due to function norm " << fnorm << " < "
            << " (relative tolerance)" << std::endl;
      reason = MooseNonlinearConvergenceReason::CONVERGED_FNORM_RELATIVE;
    }
    else if (it >= _accept_iters && checkConvergenceIndividVars(fnorm,
                                                                abstol * _accept_mult,
                                                                rtol * _accept_mult,
                                                                initial_residual_before_preset_bcs))
    {
      if (_group_resid.size() > 0)
        oss << "Converged due to function norm "
            << " < "
            << " (acceptable relative tolerance) or (acceptable absolute tolerance) for all "
               "solution variables"
            << std::endl;
      else
        oss << "Converged due to function norm " << fnorm << " < "
            << " (acceptable relative tolerance)" << std::endl;
      _console << "ACCEPTABLE" << std::endl;
      reason = MooseNonlinearConvergenceReason::CONVERGED_FNORM_RELATIVE;
    }

    else if (snorm < stol * xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << stol << " * " << xnorm
          << std::endl;
      reason = MooseNonlinearConvergenceReason::CONVERGED_SNORM_RELATIVE;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = static_cast<unsigned int>(it);

  msg = oss.str();
  return reason;
}

bool
ReferenceResidualProblem::checkConvergenceIndividVars(const Real fnorm,
                                                      const Real abstol,
                                                      const Real rtol,
                                                      const Real initial_residual_before_preset_bcs)
{
  bool convergedRelative = true;
  if (_group_resid.size() > 0)
  {
    for (unsigned int i = 0; i < _group_resid.size(); ++i)
      convergedRelative &=
          ((_group_resid[i] < _group_ref_resid[i] * rtol) || (_group_resid[i] < abstol));
  }

  else if (fnorm > initial_residual_before_preset_bcs * rtol)
    convergedRelative = false;

  return convergedRelative;
}
