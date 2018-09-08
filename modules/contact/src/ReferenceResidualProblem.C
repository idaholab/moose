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

registerMooseObject("ContactApp", ReferenceResidualProblem);

template <>
InputParameters
validParams<ReferenceResidualProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addClassDescription("Problem that checks for convergence relative to "
                             "a user-supplied reference quantity rather than "
                             "the initial residual");
  params.addParam<std::vector<std::string>>(
      "solution_variables", "Set of solution variables to be checked for relative convergence");
  params.addParam<std::vector<std::string>>(
      "reference_residual_variables",
      "Set of variables that provide reference residuals for relative convergence check");
  params.addParam<Real>("acceptable_multiplier",
                        1.0,
                        "Multiplier applied to relative tolerance for acceptable limit");
  params.addParam<int>("acceptable_iterations",
                       0,
                       "Iterations after which convergence to acceptable limits is accepted");
  params.addParam<std::vector<std::vector<std::string>>>(
      "group_variables",
      "Name of variables that are grouped together to check convergence. (Multiple groups can be "
      "provided, separated by semicolon)");
  return params;
}

ReferenceResidualProblem::ReferenceResidualProblem(const InputParameters & params)
  : FEProblem(params), _use_group_variables(false)
{
  if (params.isParamValid("solution_variables"))
    _soln_var_names = params.get<std::vector<std::string>>("solution_variables");
  if (params.isParamValid("reference_residual_variables"))
    _ref_resid_var_names = params.get<std::vector<std::string>>("reference_residual_variables");
  if (_soln_var_names.size() != _ref_resid_var_names.size())
    mooseError("In ReferenceResidualProblem, size of solution_variables (",
               _soln_var_names.size(),
               ") != size of reference_residual_variables (",
               _ref_resid_var_names.size(),
               ")");

  if (params.isParamValid("group_variables"))
  {
    _group_variables = params.get<std::vector<std::vector<std::string>>>("group_variables");
    _use_group_variables = true;
  }

  _accept_mult = params.get<Real>("acceptable_multiplier");
  _accept_iters = params.get<int>("acceptable_iterations");
}

ReferenceResidualProblem::~ReferenceResidualProblem() {}

void
ReferenceResidualProblem::addSolutionVariables(std::set<std::string> & sol_vars)
{
  for (const auto & var : sol_vars)
    _soln_var_names.push_back(var);
}

void
ReferenceResidualProblem::addReferenceResidualVariables(std::set<std::string> & ref_vars)
{
  for (const auto & var : ref_vars)
    _ref_resid_var_names.push_back(var);
}

void
ReferenceResidualProblem::addGroupVariables(std::set<std::string> & group_vars)
{
  std::vector<std::string> group_vars_vector;

  for (const auto & var : group_vars)
    group_vars_vector.push_back(var);

  _group_variables.push_back(group_vars_vector);

  _use_group_variables = true;
}

void
ReferenceResidualProblem::initialSetup()
{
  _variable_group_num_index.resize(_soln_var_names.size());

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

    unsigned int size = _soln_var_names.size() - group_variable_num + _group_variables.size();
    _group_ref_resid.resize(size);
    _group_resid.resize(size);
    _group_soln_var_names.resize(size);
    _group_ref_resid_var_names.resize(size);
  }
  else
  {
    _group_ref_resid.resize(_soln_var_names.size());
    _group_resid.resize(_soln_var_names.size());
    _group_soln_var_names.resize(_soln_var_names.size());
    _group_ref_resid_var_names.resize(_soln_var_names.size());
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

  NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
  AuxiliarySystem & aux_sys = getAuxiliarySystem();
  System & s = nonlinear_sys.system();
  TransientExplicitSystem & as = aux_sys.sys();

  if (_soln_var_names.size() > 0 && _soln_var_names.size() != s.n_vars())
    mooseError("In ReferenceResidualProblem, size of solution_variables (",
               _soln_var_names.size(),
               ") != number of variables in system (",
               s.n_vars(),
               ")");

  _soln_vars.clear();
  for (unsigned int i = 0; i < _soln_var_names.size(); ++i)
  {
    bool foundMatch = false;
    for (unsigned int var_num = 0; var_num < s.n_vars(); var_num++)
    {
      if (_soln_var_names[i] == s.variable_name(var_num))
      {
        _soln_vars.push_back(var_num);
        _resid.push_back(0.0);
        foundMatch = true;
        break;
      }
    }
    if (!foundMatch)
      mooseError("Could not find solution variable '", _soln_var_names[i], "' in system");
  }

  _ref_resid_vars.clear();
  for (unsigned int i = 0; i < _ref_resid_var_names.size(); ++i)
  {
    bool foundMatch = false;
    for (unsigned int var_num = 0; var_num < as.n_vars(); var_num++)
    {
      if (_ref_resid_var_names[i] == as.variable_name(var_num))
      {
        _ref_resid_vars.push_back(var_num);
        _ref_resid.push_back(0.0);
        foundMatch = true;
        break;
      }
    }
    if (!foundMatch)
      mooseError("Could not find variable '", _ref_resid_var_names[i], "' in auxiliary system");
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
      {
        if (std::find(_group_variables[j].begin(),
                      _group_variables[j].end(),
                      s.variable_name(_soln_vars[i])) != _group_variables[j].end())
        {
          _variable_group_num_index[i] = j;
          find_group = true;
          break;
        }
      }
      if (!find_group)
      {
        _variable_group_num_index[i] = ungroup_index;
        ungroup_index++;
      }
    }
    else
    {
      _variable_group_num_index[i] = i;
    }
  }

  if (_use_group_variables)
  {
    for (unsigned int i = 0; i < _group_variables.size(); ++i)
    {
      bool same_variable = true;
      if (_group_variables[i].size() > 1)
      {
        for (unsigned int j = 0; j < _group_variables[i].size(); ++j)
        {
          for (unsigned int var_num = 0; var_num < s.n_vars(); var_num++)
          {
            if (_group_variables[i][j] == s.variable_name(var_num))
            {
              if (j == 0)
                same_variable = nonlinear_sys.isScalarVariable(_soln_vars[var_num]);
              else
                same_variable =
                    (same_variable == nonlinear_sys.isScalarVariable(_soln_vars[var_num]) ? true
                                                                                          : false);
              break;
            }
          }
        }
      }
      if (!same_variable)
        mooseWarning("In the 'group_variables' parameter, standard variables and scalar variables "
                     "are grouped together in group ",
                     i);
    }
  }

  for (unsigned int i = 0; i < _soln_var_names.size(); ++i)
  {
    if (_group_soln_var_names[_variable_group_num_index[i]].empty())
    {
      _group_soln_var_names[_variable_group_num_index[i]] = _soln_var_names[i];
      if (_use_group_variables && _variable_group_num_index[i] < _group_variables.size())
        _group_soln_var_names[_variable_group_num_index[i]] += " (grouped) ";
    }

    if (_group_ref_resid_var_names[_variable_group_num_index[i]].empty())
    {
      _group_ref_resid_var_names[_variable_group_num_index[i]] = _ref_resid_var_names[i];
      if (_use_group_variables && _variable_group_num_index[i] < _group_variables.size())
        _group_ref_resid_var_names[_variable_group_num_index[i]] += " (grouped) ";
    }
  }

  const unsigned int size_solnVars = _soln_vars.size();
  _scaling_factors.resize(size_solnVars);
  for (unsigned int i = 0; i < size_solnVars; ++i)
    if (nonlinear_sys.isScalarVariable(_soln_vars[i]))
      _scaling_factors[i] = nonlinear_sys.getScalarVariable(0, _soln_vars[i]).scalingFactor();
    else
      _scaling_factors[i] = nonlinear_sys.getVariable(/*tid*/ 0, _soln_vars[i]).scalingFactor();

  FEProblemBase::initialSetup();
}

void
ReferenceResidualProblem::timestepSetup()
{
  for (unsigned int i = 0; i < _ref_resid.size(); ++i)
  {
    _ref_resid[i] = 0.0;
    _resid[i] = 0.0;
  }
  FEProblemBase::timestepSetup();
}

void
ReferenceResidualProblem::updateReferenceResidual()
{
  NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
  AuxiliarySystem & aux_sys = getAuxiliarySystem();
  System & s = nonlinear_sys.system();
  TransientExplicitSystem & as = aux_sys.sys();

  for (unsigned int i = 0; i < _group_resid.size(); ++i)
  {
    _group_resid[i] = 0.0;
    _group_ref_resid[i] = 0.0;
  }

  for (unsigned int i = 0; i < _soln_vars.size(); ++i)
  {
    _resid[i] = s.calculate_norm(nonlinear_sys.RHS(), _soln_vars[i], DISCRETE_L2);
    _group_resid[_variable_group_num_index[i]] += Utility::pow<2>(_resid[i]);
  }

  for (unsigned int i = 0; i < _ref_resid_vars.size(); ++i)
  {
    const Real refResidual =
        as.calculate_norm(*as.current_local_solution, _ref_resid_vars[i], DISCRETE_L2);
    _ref_resid[i] = refResidual * _scaling_factors[i];
    _group_ref_resid[_variable_group_num_index[i]] += Utility::pow<2>(_ref_resid[i]);
  }

  for (unsigned int i = 0; i < _group_resid.size(); ++i)
  {
    _group_resid[i] = sqrt(_group_resid[i]);
    _group_ref_resid[i] = sqrt(_group_ref_resid[i]);
  }
}

MooseNonlinearConvergenceReason
ReferenceResidualProblem::checkNonlinearConvergence(std::string & msg,
                                                    const PetscInt it,
                                                    const Real xnorm,
                                                    const Real snorm,
                                                    const Real fnorm,
                                                    const Real rtol,
                                                    const Real stol,
                                                    const Real abstol,
                                                    const PetscInt nfuncs,
                                                    const PetscInt max_funcs,
                                                    const PetscBool force_iteration,
                                                    const Real ref_resid,
                                                    const Real /*div_threshold*/)
{
  updateReferenceResidual();

  if (_group_soln_var_names.size() > 0)
  {
    _console << "Solution, reference convergence variable norms:" << std::endl;
    unsigned int maxwsv = 0;
    unsigned int maxwrv = 0;
    for (unsigned int i = 0; i < _group_soln_var_names.size(); ++i)
    {
      if (_group_soln_var_names[i].size() > maxwsv)
        maxwsv = _group_soln_var_names[i].size();
      if (_group_ref_resid_var_names[i].size() > maxwrv)
        maxwrv = _group_ref_resid_var_names[i].size();
    }

    for (unsigned int i = 0; i < _group_soln_var_names.size(); ++i)
      _console << std::setw(maxwsv + 2) << std::left << _group_soln_var_names[i] + ":"
               << _group_resid[i] << "  " << std::setw(maxwrv + 2)
               << _group_ref_resid_var_names[i] + ":" << _group_ref_resid[i] << std::endl;
  }

  NonlinearSystemBase & system = getNonlinearSystemBase();
  MooseNonlinearConvergenceReason reason = MOOSE_NONLINEAR_ITERATING;
  std::stringstream oss;

  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MOOSE_DIVERGED_FNORM_NAN;
  }
  else if (fnorm < abstol && (it || !force_iteration))
  {
    oss << "Converged due to function norm " << fnorm << " < " << abstol << std::endl;
    reason = MOOSE_CONVERGED_FNORM_ABS;
  }
  else if (nfuncs >= max_funcs)
  {
    oss << "Exceeded maximum number of function evaluations: " << nfuncs << " > " << max_funcs
        << std::endl;
    reason = MOOSE_DIVERGED_FUNCTION_COUNT;
  }

  if (it && !reason)
  {
    if (checkConvergenceIndividVars(fnorm, abstol, rtol, ref_resid))
    {
      if (_group_resid.size() > 0)
        oss << "Converged due to function norm "
            << " < "
            << " (relative tolerance) or (absolute tolerance) for all solution variables"
            << std::endl;
      else
        oss << "Converged due to function norm " << fnorm << " < "
            << " (relative tolerance)" << std::endl;
      reason = MOOSE_CONVERGED_FNORM_RELATIVE;
    }
    else if (it >= _accept_iters &&
             checkConvergenceIndividVars(
                 fnorm, abstol * _accept_mult, rtol * _accept_mult, ref_resid))
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
      reason = MOOSE_CONVERGED_FNORM_RELATIVE;
    }

    else if (snorm < stol * xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << stol << " * " << xnorm
          << std::endl;
      reason = MOOSE_CONVERGED_SNORM_RELATIVE;
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
                                                      const Real ref_resid)
{
  bool convergedRelative = true;
  if (_group_resid.size() > 0)
  {
    for (unsigned int i = 0; i < _group_resid.size(); ++i)
      convergedRelative &=
          ((_group_resid[i] < _group_ref_resid[i] * rtol) || (_group_resid[i] < abstol));
  }

  else if (fnorm > ref_resid * rtol)
    convergedRelative = false;

  return convergedRelative;
}
