/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "ReferenceResidualProblem.h"

#include "AuxiliarySystem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<ReferenceResidualProblem>()
{
  InputParameters params = validParams<FEProblem>();
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
  return params;
}

ReferenceResidualProblem::ReferenceResidualProblem(const InputParameters & params)
  : FEProblem(params)
{
  if (params.isParamValid("solution_variables"))
    _solnVarNames = params.get<std::vector<std::string>>("solution_variables");
  if (params.isParamValid("reference_residual_variables"))
    _refResidVarNames = params.get<std::vector<std::string>>("reference_residual_variables");
  if (_solnVarNames.size() != _refResidVarNames.size())
    mooseError("In ReferenceResidualProblem, size of solution_variables (",
               _solnVarNames.size(),
               ") != size of reference_residual_variables (",
               _refResidVarNames.size(),
               ")");

  _accept_mult = params.get<Real>("acceptable_multiplier");
  _accept_iters = params.get<int>("acceptable_iterations");
}

ReferenceResidualProblem::~ReferenceResidualProblem() {}

void
ReferenceResidualProblem::initialSetup()
{
  NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
  AuxiliarySystem & aux_sys = getAuxiliarySystem();
  System & s = nonlinear_sys.system();
  TransientExplicitSystem & as = aux_sys.sys();

  if (_solnVarNames.size() > 0 && _solnVarNames.size() != s.n_vars())
    mooseError("In ReferenceResidualProblem, size of solution_variables (",
               _solnVarNames.size(),
               ") != number of variables in system (",
               s.n_vars(),
               ")");

  _solnVars.clear();
  for (unsigned int i = 0; i < _solnVarNames.size(); ++i)
  {
    bool foundMatch = false;
    for (unsigned int var_num = 0; var_num < s.n_vars(); var_num++)
    {
      if (_solnVarNames[i] == s.variable_name(var_num))
      {
        _solnVars.push_back(var_num);
        _resid.push_back(0.0);
        foundMatch = true;
        break;
      }
    }
    if (!foundMatch)
      mooseError("Could not find solution variable '", _solnVarNames[i], "' in system");
  }

  _refResidVars.clear();
  for (unsigned int i = 0; i < _refResidVarNames.size(); ++i)
  {
    bool foundMatch = false;
    for (unsigned int var_num = 0; var_num < as.n_vars(); var_num++)
    {
      if (_refResidVarNames[i] == as.variable_name(var_num))
      {
        _refResidVars.push_back(var_num);
        _refResid.push_back(0.0);
        foundMatch = true;
        break;
      }
    }
    if (!foundMatch)
      mooseError("Could not find variable '", _refResidVarNames[i], "' in auxiliary system");
  }

  FEProblemBase::initialSetup();
}

void
ReferenceResidualProblem::timestepSetup()
{
  for (unsigned int i = 0; i < _refResid.size(); ++i)
  {
    _refResid[i] = 0.0;
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

  for (unsigned int i = 0; i < _solnVars.size(); ++i)
    _resid[i] = s.calculate_norm(nonlinear_sys.RHS(), _solnVars[i], DISCRETE_L2);

  for (unsigned int i = 0; i < _refResidVars.size(); ++i)
    _refResid[i] = as.calculate_norm(*as.current_local_solution, _refResidVars[i], DISCRETE_L2);
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
                                                    const Real ref_resid,
                                                    const Real /*div_threshold*/)
{
  updateReferenceResidual();

  if (_solnVars.size() > 0)
  {
    _console << "Solution, reference convergence variable norms:" << std::endl;
    unsigned int maxwsv = 0;
    unsigned int maxwrv = 0;
    for (unsigned int i = 0; i < _solnVars.size(); ++i)
    {
      if (_solnVarNames[i].size() > maxwsv)
        maxwsv = _solnVarNames[i].size();
      if (_refResidVarNames[i].size() > maxwrv)
        maxwrv = _refResidVarNames[i].size();
    }

    for (unsigned int i = 0; i < _solnVars.size(); ++i)
      _console << std::setw(maxwsv + 2) << std::left << _solnVarNames[i] + ":" << _resid[i] << "  "
               << std::setw(maxwrv + 2) << _refResidVarNames[i] + ":" << _refResid[i] << std::endl;
  }

  NonlinearSystemBase & system = getNonlinearSystemBase();
  MooseNonlinearConvergenceReason reason = MOOSE_NONLINEAR_ITERATING;
  std::stringstream oss;

  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MOOSE_DIVERGED_FNORM_NAN;
  }
  else if (fnorm < abstol)
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
      if (_resid.size() > 0)
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
      if (_resid.size() > 0)
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
  if (_resid.size() > 0)
  {
    for (unsigned int i = 0; i < _resid.size(); ++i)
      convergedRelative &= ((_resid[i] < _refResid[i] * rtol) || (_resid[i] < abstol));
  }

  else if (fnorm > ref_resid * rtol)
    convergedRelative = false;

  return convergedRelative;
}
