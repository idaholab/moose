//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ReferenceResidualConvergence.h"
#include "ReferenceResidualProblem.h"
#include "FEProblemBase.h"
#include "PetscSupport.h"
#include "Executioner.h"
#include "PerfGraphInterface.h"
#include "NonlinearSystemBase.h"
#include "ActionWarehouse.h"

#include "AuxiliarySystem.h"
//#include "MooseApp.h"
//#include "MooseMesh.h"
//#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "NonlinearSystem.h"

// PETSc includes
#include <petscsnes.h>
#include <petscksp.h>
#include <petscdm.h>

// PetscDMMoose include
#include "PetscDMMoose.h"

registerMooseObject("MooseApp", ReferenceResidualConvergence);

InputParameters
ReferenceResidualConvergence::validParams()
{
  InputParameters params = Convergence::validParams();
  params += FEProblemSolve::commonParams();
  params += ReferenceResidualProblem::commonParams();

  params.addClassDescription("Check ReferenceResidualConvergence of the set up problem.");

  return params;
}

ReferenceResidualConvergence::ReferenceResidualConvergence(const InputParameters & parameters)
  : Convergence(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _perf_nonlinear(
        registerTimedSection("checkNonlinearConvergence", 5, "Checking Nonlinear Convergence")),
    _use_group_variables(false),
    _zero_ref_type(parameters.get<MooseEnum>("zero_reference_residual_treatment")
                       .getEnum<ZeroReferenceType>()),
    _reference_vector(nullptr),
    _converge_on(getParam<std::vector<NonlinearVariableName>>("converge_on")),
    _reference_vector_tag_id(Moose::INVALID_TAG_ID),
    _initialized(false)
{
  EquationSystems & es = _fe_problem.es();

  if (isParamSetByUser("l_tol"))
  {
    es.parameters.set<Real>("linear solver tolerance") = getParam<Real>("l_tol");
    _l_tol = getParam<Real>("l_tol");
  }

  if (isParamSetByUser("l_abs_tol"))
  {
    es.parameters.set<Real>("linear solver absolute tolerance") = getParam<Real>("l_abs_tol");
    _l_abs_tol = getParam<Real>("l_abs_tol");
  }

  if (isParamSetByUser("l_max_its"))
  {
    es.parameters.set<unsigned int>("linear solver maximum iterations") =
        getParam<unsigned int>("l_max_its");
    _l_max_its = getParam<unsigned int>("l_max_its");
  }

  if (isParamSetByUser("nl_max_its"))
  {
    es.parameters.set<unsigned int>("nonlinear solver maximum iterations") =
        getParam<unsigned int>("nl_max_its");
    _nl_max_its = getParam<unsigned int>("nl_max_its");
  }

  if (isParamSetByUser("nl_forced_its"))
  {
    _fe_problem.setNonlinearForcedIterations(getParam<unsigned int>("nl_forced_its"));
    _nl_forced_its = getParam<unsigned int>("nl_forced_its");
  }

  if (isParamSetByUser("nl_max_funcs"))
  {
    es.parameters.set<unsigned int>("nonlinear solver maximum function evaluations") =
        getParam<unsigned int>("nl_max_funcs");
    _nl_max_funcs = getParam<unsigned int>("nl_max_funcs");
  }

  if (isParamSetByUser("nl_abs_tol"))
  {
    es.parameters.set<Real>("nonlinear solver absolute residual tolerance") =
        getParam<Real>("nl_abs_tol");
    _nl_abs_tol = getParam<Real>("nl_abs_tol");
  }

  if (isParamSetByUser("nl_rel_tol"))
  {
    es.parameters.set<Real>("nonlinear solver relative residual tolerance") =
        getParam<Real>("nl_rel_tol");
    _nl_rel_tol = getParam<Real>("nl_rel_tol");
  }

  if (isParamSetByUser("nl_div_tol"))
  {
    es.parameters.set<Real>("nonlinear solver divergence tolerance") = getParam<Real>("nl_div_tol");
    _divtol = getParam<Real>("nl_div_tol");
  }

  if (isParamSetByUser("nl_abs_div_tol"))
    _fe_problem.setNonlinearAbsoluteDivergenceTolerance(getParam<Real>("nl_abs_div_tol"));

  if (isParamSetByUser("nl_abs_step_tol"))
  {
    es.parameters.set<Real>("nonlinear solver absolute step tolerance") =
        getParam<Real>("nl_abs_step_tol");
    _nl_abs_step_tol = getParam<Real>("nl_abs_step_tol");
  }

  if (isParamSetByUser("nl_rel_step_tol"))
  {
    es.parameters.set<Real>("nonlinear solver relative step tolerance") =
        getParam<Real>("nl_rel_step_tol");
    _nl_rel_step_tol = getParam<Real>("nl_rel_step_tol");
  }

  if (isParamSetByUser("nl_max_nonlinear_pingpong"))
  {
    _fe_problem.setMaxNLPingPong(getParam<unsigned int>("n_max_nonlinear_pingpong"));
    _n_max_nl_pingpong = getParam<unsigned int>("n_max_nonlinear_pingpong");
  }

  ///The following parameters are from ReferenceResidualProblem

  const auto ref_problem = dynamic_cast<ReferenceResidualProblem *>(&_fe_problem);

  if (parameters.isParamSetByUser("solution_variables"))
  {
    if (parameters.isParamSetByUser("reference_vector"))
      mooseDeprecated("The `solution_variables` parameter is deprecated, has no effect when "
                      "the tagging system is used, and will be removed on January 1, 2020. "
                      "Please simply delete this parameter from your input file.");
    _soln_var_names = parameters.get<std::vector<NonlinearVariableName>>("solution_variables");
  }

  if (parameters.isParamSetByUser("reference_residual_variables") &&
      parameters.isParamSetByUser("reference_vector"))
    mooseError(
        "For `ReferenceResidualProblem` you can specify either the `reference_residual_variables` "
        "or `reference_vector` parameter, not both. `reference_residual_variables` is deprecated "
        "so we recommend using `reference_vector`");

  if (parameters.isParamSetByUser("reference_residual_variables"))
  {
    mooseDeprecated(
        "The save-in method for composing reference residual quantities is deprecated "
        "and will be removed on January 1, 2020. Please use the tagging system instead; "
        "specifically, please assign a TagName to the `reference_vector` parameter");

    _ref_resid_var_names =
        parameters.get<std::vector<AuxVariableName>>("reference_residual_variables");

    if (_soln_var_names.size() != _ref_resid_var_names.size())
      mooseError("In ReferenceResidualProblem, size of solution_variables (",
                 _soln_var_names.size(),
                 ") != size of reference_residual_variables (",
                 _ref_resid_var_names.size(),
                 ")");
  }

  if (parameters.isParamSetByUser("reference_vector"))
  {
    if (_fe_problem.numNonlinearSystems() > 1)
      paramError(
          "nl_sys_names",
          "reference residual problem does not currently support multiple nonlinear systems");
    _reference_vector_tag_id = _fe_problem.getVectorTagID(getParam<TagName>("reference_vector"));
    _reference_vector = &_fe_problem.getNonlinearSystemBase(0).getVector(_reference_vector_tag_id);
  }
  else
  {
    _reference_vector_tag_id = _fe_problem.getVectorTagID(ref_problem->getReferenceVectorTag());
    _reference_vector = &_fe_problem.getNonlinearSystemBase(0).getVector(_reference_vector_tag_id);
  }

  if (parameters.isParamValid("reference_residual_variables") ||
      parameters.isParamValid("reference_vector"))
    mooseInfo("Neither the `reference_residual_variables` nor `reference_vector` parameter is "
              "specified for `ReferenceResidualProblem`, which means that no reference "
              "quantites are set. Because of this, the standard technique of comparing the "
              "norm of the full residual vector with its initial value will be used.");

  if (parameters.isParamSetByUser("group_variables"))
  {
    _group_variables =
        parameters.get<std::vector<std::vector<NonlinearVariableName>>>("group_variables");
    _use_group_variables = true;
  }
  else
  {
    _group_variables = ref_problem->getGroupVars();
    if (_group_variables.size() > 0)
      _use_group_variables = true;
  }

  if (parameters.isParamSetByUser("acceptable_multiplier"))
    _accept_mult = parameters.get<Real>("acceptable_multiplier");
  else
    _accept_mult = ref_problem->getAcceptableMultipliers();
  if (parameters.isParamSetByUser("acceptable_iterations"))
    _accept_iters = parameters.get<int>("acceptable_iterations");
  else
    _accept_iters = ref_problem->getAcceptableIterations();

  int castEnum = ref_problem->getNormType();
  auto norm_type_enum = NormalizationType(castEnum);
  if (parameters.isParamSetByUser("normalization_type"))
  {
    norm_type_enum = parameters.get<MooseEnum>("normalization_type").getEnum<NormalizationType>();
  }

  if (norm_type_enum == NormalizationType::LOCAL_L2)
  {
    _norm_type = DISCRETE_L2;
    _local_norm = true;
  }
  else if (norm_type_enum == NormalizationType::GLOBAL_L2)
  {
    _norm_type = DISCRETE_L2;
    _local_norm = false;
  }
  else if (norm_type_enum == NormalizationType::LOCAL_LINF)
  {
    _norm_type = DISCRETE_L_INF;
    _local_norm = true;
  }
  else if (norm_type_enum == NormalizationType::GLOBAL_LINF)
  {
    _norm_type = DISCRETE_L_INF;
    _local_norm = false;
  }
  else
    mooseError("Internal error");

  if (_local_norm && !parameters.isParamSetByUser("reference_vector"))
    paramError("reference_vector", "If local norm is used, a reference_vector must be provided.");
}

void
ReferenceResidualConvergence::setupReferenceResidual()
{
  NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase(/*nl_sys=*/0);
  AuxiliarySystem & aux_sys = _fe_problem.getAuxiliarySystem();
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

  _fe_problem.initialSetup();
}

void
ReferenceResidualConvergence::updateReferenceResidual()
{
  // mooseAssert(_current_nl_sys, "This should be non-null");
  NonlinearSystemBase & _current_nl_sys = _fe_problem.currentNonlinearSystem();
  AuxiliarySystem & aux_sys = _fe_problem.getAuxiliarySystem();
  System & s = _current_nl_sys.system();
  auto & as = aux_sys.sys();

  std::fill(_group_resid.begin(), _group_resid.end(), 0.0);
  std::fill(_group_output_resid.begin(), _group_output_resid.end(), 0.0);
  if (_local_norm)
    std::fill(_group_ref_resid.begin(), _group_ref_resid.end(), 1.0);
  else
    std::fill(_group_ref_resid.begin(), _group_ref_resid.end(), 0.0);

  for (unsigned int i = 0; i < _soln_vars.size(); ++i)
  {
    Real resid = 0.0;
    const auto group = _variable_group_num_index[i];
    if (_local_norm)
    {
      mooseAssert(_current_nl_sys.RHS().size() == (*_reference_vector).size(),
                  "Sizes of nonlinear RHS and reference vector should be the same.");
      mooseAssert((*_reference_vector).size(), "Reference vector must be provided.");
      // Add a tiny number to the reference to prevent a divide by zero.
      auto ref = _reference_vector->clone();
      ref->add(std::numeric_limits<Number>::min());
      auto div = _current_nl_sys.RHS().clone();
      *div /= *ref;
      resid = Utility::pow<2>(s.calculate_norm(*div, _soln_vars[i], _norm_type));
    }
    else
    {
      resid = Utility::pow<2>(s.calculate_norm(_current_nl_sys.RHS(), _soln_vars[i], _norm_type));
      if (_reference_vector)
      {
        const auto ref_resid = s.calculate_norm(*_reference_vector, _soln_vars[i], _norm_type);
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
          as.calculate_norm(*as.current_local_solution, _ref_resid_vars[i], _norm_type) *
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

void
ReferenceResidualConvergence::nonlinearConvergenceSetup()
{
  updateReferenceResidual();

  std::ostringstream out;

  if (_group_soln_var_names.size() > 0)
  {
    out << std::setprecision(2) << std::scientific
        << "   Solution, reference convergence variable norms:\n";
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
      out << "   " << std::setw(maxwsv + (_local_norm ? 5 : 2)) << std::left
          << (_local_norm ? "norm " : "") + _group_soln_var_names[i] + ": ";

      if (_group_output_resid[i] == _group_resid[i])
        out << std::setw(8) << _group_output_resid[i];
      else
        out << std::setw(8) << _group_resid[i] << " (" << _group_output_resid[i] << ')';

      if (!_local_norm)
      {
        const auto ref_var_name =
            _reference_vector ? _group_soln_var_names[i] + "_ref" : _group_ref_resid_var_names[i];
        out << "  " << std::setw(maxwrv + 2) << ref_var_name + ":" << std::setw(8)
            << _group_ref_resid[i] << "  (" << std::setw(8)
            << (_group_ref_resid[i] ? _group_resid[i] / _group_ref_resid[i] : _group_resid[i])
            << ")";
      }
      out << '\n';
    }

    _console << out.str() << std::flush;
  }
}

bool
ReferenceResidualConvergence::checkConvergenceIndividVars(
    const Real fnorm,
    const Real abstol,
    const Real rtol,
    const Real initial_residual_before_preset_bcs)
{
  // Convergence is checked via:
  // 1) if group residual is less than group reference residual by relative tolerance
  // 2) if group residual is less than absolute tolerance
  // 3) if group reference residual is zero and:
  //   3.1) Convergence type is ZERO_TOLERANCE and group residual is zero (rare, but possible, and
  //        historically implemented way)
  //   3.2) Convergence type is RELATIVE_TOLERANCE and group residual
  //        is less than relative tolerance. (i.e., using the relative tolerance to check group
  //        convergence in an absolute way)

  bool convergedRelative = true;
  if (_group_resid.size() > 0)
  {
    for (unsigned int i = 0; i < _group_resid.size(); ++i)
      convergedRelative &=
          (_group_resid[i] < _group_ref_resid[i] * rtol || _group_resid[i] < abstol ||
           (_group_ref_resid[i] == 0.0 &&
            ((_zero_ref_type == ZeroReferenceType::ZERO_TOLERANCE && _group_resid[i] == 0.0) ||
             (_zero_ref_type == ZeroReferenceType::RELATIVE_TOLERANCE &&
              _group_resid[i] <= rtol))));
  }

  else if (fnorm > initial_residual_before_preset_bcs * rtol)
    convergedRelative = false;

  return convergedRelative;
}

bool
ReferenceResidualConvergence::checkRelativeConvergence(const PetscInt it,
                                                       const Real fnorm,
                                                       const Real the_residual,
                                                       const Real rtol,
                                                       const Real abstol,
                                                       std::ostringstream & oss)
{
  if (checkConvergenceIndividVars(fnorm, abstol, rtol, the_residual))
  {
    oss << "Converged due to function norm " << fnorm << " < relative tolerance (" << rtol
        << ") or absolute tolerance (" << abstol << ") for all solution variables\n";
    return true;
  }
  else if (it >= _accept_iters &&
           checkConvergenceIndividVars(
               fnorm, abstol * _accept_mult, rtol * _accept_mult, the_residual))
  {
    oss << "Converged due to function norm " << fnorm << " < acceptable relative tolerance ("
        << rtol * _accept_mult << ") or acceptable absolute tolerance (" << abstol * _accept_mult
        << ") for all solution variables\n";
    _console << "Converged due to ACCEPTABLE tolerances" << std::endl;
    return true;
  }

  return false;
}

Convergence::MooseAlgebraicConvergence
ReferenceResidualConvergence::checkAlgebraicConvergence(int it, Real xnorm, Real snorm, Real fnorm)
{
  TIME_SECTION("checkNonlinearConvergence", 5, "Checking Algebraic Convergence");

  MooseAlgebraicConvergence reason;
  nonlinearConvergenceSetup();
  // To check if the nonlinear iterations should abort
  bool terminate = _fe_problem.getFailNextNonlinearConvergenceCheck();
  if (terminate)
  {
    _fe_problem.resetFailNextNonlinearConvergenceCheck();
    reason = MooseAlgebraicConvergence::DIVERGED;
  }

  NonlinearSystemBase & system = _fe_problem.currentNonlinearSystem();
  reason = MooseAlgebraicConvergence::ITERATING;

  // To check PETSc error codes.
  PetscErrorCode ierr = 0;
  SNES snes = system.getSNES();

  // Ask the SNES object about its tolerances.
  ierr = SNESGetTolerances(snes, &_atol, &_rtol, &_stol, &_maxit, &_maxf);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

  // Ask the SNES object about its divergence tolerance
#if !PETSC_VERSION_LESS_THAN(3, 8, 0)
  ierr = SNESGetDivergenceTolerance(snes, &_divtol);
  CHKERRABORT(_fe_problem.comm().get(), ierr);
#endif

  // Get current number of function evaluations done by SNES.
  ierr = SNESGetNumberFunctionEvals(snes, &_nfuncs);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

  // Whether or not to force SNESSolve() take at least one iteration regardless of the initial
  // residual norm
#if !PETSC_VERSION_LESS_THAN(3, 8, 4)
  PetscBool force_iteration = PETSC_FALSE;
  ierr = SNESGetForceIteration(snes, &force_iteration);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

  if (force_iteration && !(_fe_problem.getNonlinearForcedIterations()))
    _fe_problem.setNonlinearForcedIterations(1);

  if (!force_iteration && (_fe_problem.getNonlinearForcedIterations()))
  {
    ierr = SNESSetForceIteration(snes, PETSC_TRUE);
    CHKERRABORT(_fe_problem.comm().get(), ierr);
  }
#endif

  // See if SNESSetFunctionDomainError() has been called.  Note:
  // SNESSetFunctionDomainError() and SNESGetFunctionDomainError()
  // were added in different releases of PETSc.
  PetscBool domainerror;
  ierr = SNESGetFunctionDomainError(snes, &domainerror);
  CHKERRABORT(_fe_problem.comm().get(), ierr);
  if (domainerror)
  {
    reason = MooseAlgebraicConvergence::DIVERGED;
  }

  Real fnorm_old;

  _nl_abs_div_tol = _fe_problem.getNonlinearAbsoluteDivergenceTolerance();

  // This is the first residual before any iterations have been done,
  // but after preset BCs (if any) have been imposed on the solution
  // vector.  We save it, and use it to detect convergence if
  // compute_initial_residual_before_preset_bcs=false.
  if (it == 0)
  {
    system._initial_residual_after_preset_bcs = fnorm;
    fnorm_old = fnorm;
    _n_nl_pingpong = 0;
  }
  else
    fnorm_old = system._last_nl_rnorm;

  // Check for nonlinear residual pingpong.
  // Pingpong will always start from a residual increase
  if ((_n_nl_pingpong % 2 == 1 && !(fnorm > fnorm_old)) ||
      (_n_nl_pingpong % 2 == 0 && fnorm > fnorm_old))
    _n_nl_pingpong += 1;
  else
    _n_nl_pingpong = 0;

  long int _nl_forced_its = _fe_problem.getNonlinearForcedIterations();

  std::ostringstream oss;
  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MooseAlgebraicConvergence::DIVERGED;
  }
  else if ((it >= _nl_forced_its) && fnorm < _atol)
  {
    oss << "Converged due to function norm " << fnorm << " < " << _atol << '\n';
    reason = MooseAlgebraicConvergence::CONVERGED;
  }
  else if (_nfuncs >= _maxf)
  {
    oss << "Exceeded maximum number of function evaluations: " << _nfuncs << " > " << _maxf << '\n';
    reason = MooseAlgebraicConvergence::DIVERGED;
  }
  else if ((it >= _nl_forced_its) && it && fnorm > system._last_nl_rnorm && fnorm >= _div_threshold)
  {
    oss << "Nonlinear solve was blowing up!\n";
    reason = MooseAlgebraicConvergence::DIVERGED;
  }
  if ((it >= _nl_forced_its) && it && reason == MooseAlgebraicConvergence::ITERATING)
  {
    // If compute_initial_residual_before_preset_bcs==false, then use the
    // first residual computed by PETSc to determine convergence.
    Real the_residual = system._compute_initial_residual_before_preset_bcs
                            ? system._initial_residual_before_preset_bcs
                            : system._initial_residual_after_preset_bcs;
    if (checkRelativeConvergence(it, fnorm, the_residual, _rtol, _atol, oss))
      reason = MooseAlgebraicConvergence::CONVERGED;
    else if (snorm < _stol * xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << _stol << " * " << xnorm
          << '\n';
      reason = MooseAlgebraicConvergence::CONVERGED;
    }
    else if (_divtol > 0 && fnorm > the_residual * _divtol)
    {
      oss << "Diverged due to initial residual " << the_residual << " > divergence tolerance "
          << _divtol << " * initial residual " << the_residual << '\n';
      reason = MooseAlgebraicConvergence::DIVERGED;
    }
    else if (_nl_abs_div_tol > 0 && fnorm > _nl_abs_div_tol)
    {
      oss << "Diverged due to residual " << fnorm << " > absolute divergence tolerance "
          << _nl_abs_div_tol << '\n';
      reason = MooseAlgebraicConvergence::DIVERGED;
    }
    else if (_n_nl_pingpong > _n_max_nl_pingpong)
    {
      oss << "Diverged due to maximum nonlinear residual pingpong achieved" << '\n';
      reason = MooseAlgebraicConvergence::DIVERGED;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = static_cast<unsigned int>(it);

  std::string msg;
  msg = oss.str();
  if (_app.multiAppLevel() > 0)
    MooseUtils::indentMessage(_app.name(), msg);
  if (msg.length() > 0)
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
    PetscInfo(snes, "%s", msg.c_str());
#else
    PetscInfo(snes, msg.c_str());
#endif

  return reason;
}
