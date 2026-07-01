//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "NonlinearSystemBase.h"
#include "TaggingInterface.h"
#include "AuxiliarySystem.h"
#include "MooseVariableScalar.h"
#include "NonlinearSystem.h"

// PETSc includes
#include <petscsnes.h>

registerMooseObject("MooseApp", ReferenceResidualConvergence);

InputParameters
ReferenceResidualConvergence::validParams()
{
  InputParameters params = DefaultNonlinearConvergence::validParams();
  params += ReferenceResidualInterface::validParams();

  params.addClassDescription(
      "Check the convergence of a problem with respect to a user-supplied reference solution."
      " Replaces ReferenceResidualProblem, currently still used in conjunction with it.");

  return params;
}

ReferenceResidualConvergence::ReferenceResidualConvergence(const InputParameters & parameters)
  : DefaultNonlinearConvergence(parameters),
    ReferenceResidualInterface(this),
    _norm_type_enum(getParam<MooseEnum>("normalization_type")),
    _accept_mult(getParam<Real>("acceptable_multiplier")),
    _accept_iters(getParam<unsigned int>("acceptable_iterations")),
    _residual_vector(nullptr),
    _reference_vector(nullptr),
    _zero_ref_type(
        getParam<MooseEnum>("zero_reference_residual_treatment").getEnum<ZeroReferenceType>()),
    _unscale_the_residual(getParam<bool>("unscale_the_residual")),
    _reference_vector_tag_id(Moose::INVALID_TAG_ID)
{
  // This restriction is primarily due to reference and residual vector parameters
  if (_fe_problem.numNonlinearSystems() > 1)
    paramError("nl_sys_names",
               "reference residual problem does not currently support multiple nonlinear systems");

  if (parameters.isParamValid("residual_vector"))
  {
    const auto residual_vector_tag_id =
        _fe_problem.getVectorTagID(getParam<TagName>("residual_vector"));
    _residual_vector = &_fe_problem.getNonlinearSystemBase(0).getVector(residual_vector_tag_id);
  }
  else
    _residual_vector = &_fe_problem.getNonlinearSystemBase(0).RHS();

  if (parameters.isParamValid("reference_vector"))
  {
    _reference_vector_tag_id = _fe_problem.getVectorTagID(getParam<TagName>("reference_vector"));
    _reference_vector = &_fe_problem.getNonlinearSystemBase(0).getVector(_reference_vector_tag_id);
  }
  else
    mooseDeprecated(
        "No `reference_vector` is provided, thus the Reference Residual convergence method will "
        "revert to default tolerance checking. `reference_vector` will become a required parameter "
        "on June 1st, 2027. If you are using `ReferenceResidualProblem`, either provide a "
        "reference_vector or use a standard problem type (e.g., remove "
        "Problem/type=ReferenceResidualProblem from your input file). If you are using "
        "`ReferenceResidualConvergence`, either provide a reference_vector or utilize "
        "`DefaultNonlinearConvergence` instead.");

  if (_norm_type_enum == "LOCAL_L2")
  {
    _norm_type = libMesh::DISCRETE_L2;
    _local_norm = true;
  }
  else if (_norm_type_enum == "GLOBAL_L2")
  {
    _norm_type = libMesh::DISCRETE_L2;
    _local_norm = false;
  }
  else if (_norm_type_enum == "LOCAL_LINF")
  {
    _norm_type = libMesh::DISCRETE_L_INF;
    _local_norm = true;
  }
  else if (_norm_type_enum == "GLOBAL_LINF")
  {
    _norm_type = libMesh::DISCRETE_L_INF;
    _local_norm = false;
  }
  else
    mooseAssert(false, "This point should not be reached.");

  if (_local_norm && !parameters.isParamValid("reference_vector"))
    paramError("reference_vector", "If local norm is used, a reference_vector must be provided.");
}

void
ReferenceResidualConvergence::initialSetup()
{
  DefaultNonlinearConvergence::initialSetup();
  // If no refernce_vector is provided, just revert to DefaultNonlinearConvergence behavior
  if (!_reference_vector)
    return;

  auto & nonlinear_sys = _fe_problem.getNonlinearSystemBase(/*nl_sys=*/0);
  auto & s = nonlinear_sys.system();

  // If the user provides reference_vector, that implies that they want the
  // individual variables compared against their reference quantities in the
  // tag vector. The code depends on having _soln_var_names populated,
  // so fill that out if they didn't specify solution_variables.
  for (const auto var_num : make_range(s.n_vars()))
    _soln_var_names.push_back(s.variable_name(var_num));
  const auto n_soln_vars = nonlinear_sys.nVariables();

  const auto converge_on = getParam<std::vector<NonlinearVariableName>>("converge_on");
  if (!converge_on.empty())
  {
    _converge_on_var.assign(n_soln_vars, false);
    for (std::size_t i = 0; i < n_soln_vars; ++i)
      for (const auto & c : converge_on)
        if (MooseUtils::globCompare(_soln_var_names[i], c))
        {
          _converge_on_var[i] = true;
          break;
        }
  }
  else
    _converge_on_var.assign(n_soln_vars, true);

  unsigned int num_variables_in_groups = 0;
  for (const auto i : index_range(_group_variables))
  {
    num_variables_in_groups += _group_variables[i].size();
    if (_group_variables[i].size() == 1)
      paramError("group_variables",
                 "variable ",
                 _group_variables[i][0],
                 " is not grouped with other variables.");
  }

  // If no groups, size = n_soln_vars
  unsigned int n_groups = n_soln_vars - num_variables_in_groups + _group_variables.size();
  _group_ref_resid.resize(n_groups);
  _group_resid.resize(n_groups);
  _group_names.resize(n_groups);
  _converge_on_group.assign(n_groups, true);
  _scaling_factors.resize(n_soln_vars);

  // Check to make sure variables aren't in multiple groups
  if (_use_group_variables)
  {
    std::set<std::string> check_duplicate;
    for (const auto i : index_range(_group_variables))
      for (const auto j : index_range(_group_variables[i]))
        check_duplicate.insert(_group_variables[i][j]);

    if (check_duplicate.size() != num_variables_in_groups)
      paramError("group_variables", "A variable cannot be included in multiple groups.");
  }

  _soln_vars.clear();
  for (const auto i : make_range(n_soln_vars))
  {
    bool found_match = false;
    for (const auto var_num : make_range(s.n_vars()))
      if (_soln_var_names[i] == s.variable_name(var_num))
      {
        _soln_vars.push_back(var_num);
        found_match = true;
        break;
      }

    if (!found_match)
      mooseError("Could not find solution variable '",
                 _soln_var_names[i],
                 "' in system '",
                 s.name(),
                 "'.");
  }

  unsigned int ungroup_index = 0;
  if (_use_group_variables)
    ungroup_index = _group_variables.size();

  // Determine which group each variable belongs to
  _group_index.resize(n_soln_vars);
  _is_var_grouped.assign(n_soln_vars, false);
  for (const auto i : index_range(_soln_vars))
  {
    if (_use_group_variables)
    {
      for (const auto j : index_range(_group_variables))
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

          _group_index[i] = j;
          _is_var_grouped[i] = true;
          break;
        }

      if (!_is_var_grouped[i])
      {
        _group_index[i] = ungroup_index;
        ungroup_index++;
      }
    }
    else
      _group_index[i] = i;
  }

  // Check for variable groups containing both field and scalar variables
  for (const auto i : index_range(_group_variables))
  {
    unsigned int num_scalar_vars = 0;
    unsigned int num_field_vars = 0;
    if (_group_variables[i].size() > 1)
    {
      for (const auto j : index_range(_group_variables[i]))
        for (const auto var_num : make_range(s.n_vars()))
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
      paramWarning("group_variables",
                   "standard variables and scalar variables are grouped together in group ",
                   i);
  }

  for (const auto i : index_range(_group_names))
  {
    // Accumulate names for a given group
    std::vector<NonlinearVariableName> names;
    for (const auto j : index_range(_group_index))
      if (_group_index[j] == i)
      {
        names.push_back(_soln_var_names[j]);
        _converge_on_group[i] = _converge_on_group[i] && _converge_on_var[j];
      }
    if (names.size() == 0)
      mooseError("Internal error, something is wrong with variable grouping");
    else if (names.size() == 1)
      _group_names[i] = names[0];
    else
    {
      _group_names[i] = "(";
      for (const auto j : index_range(names))
      {
        _group_names[i] += names[j];
        if (j != names.size() - 1)
          _group_names[i] += ", ";
      }
      _group_names[i] += ")";
    }
  }
}

void
ReferenceResidualConvergence::updateReferenceResidual()
{
  // If no reference_vector is provided, this method is completely skipped

  auto & current_nl_sys = _fe_problem.currentNonlinearSystem();
  auto & s = current_nl_sys.system();

  for (const auto i : index_range(_scaling_factors))
    if (current_nl_sys.isScalarVariable(_soln_vars[i]))
      _scaling_factors[i] = current_nl_sys.getScalarVariable(0, _soln_vars[i]).scalingFactor();
    else
      _scaling_factors[i] = current_nl_sys.getVariable(/*tid*/ 0, _soln_vars[i]).scalingFactor();

  std::fill(_group_resid.begin(), _group_resid.end(), 0.0);
  std::fill(_group_ref_resid.begin(), _group_ref_resid.end(), 0.0);

  for (const auto i : index_range(_soln_vars))
  {
    if (_converge_on_var[i])
    {
      const auto group = _group_index[i];

      // Prepare residual
      auto resid = Utility::pow<2>(s.calculate_norm(*_residual_vector, _soln_vars[i], _norm_type));
      if (_unscale_the_residual)
      {
        mooseAssert(_scaling_factors[i], "Scaling factor must not be zero");
        resid /= Utility::pow<2>(_scaling_factors[i]);
      }
      _group_resid[group] += resid;

      // Prepare reference residual. If local norm, this is actually the ratio of the residual
      // dividied by the reference at all DOF
      Real ref_resid;
      if (_local_norm)
      {
        mooseAssert((*_residual_vector).size() == (*_reference_vector).size(),
                    "Sizes of nonlinear RHS and reference vector should be the same.");
        mooseAssert((*_reference_vector).size(), "Reference vector must be provided.");
        auto ref = _reference_vector->clone();
        // Add a tiny number to the reference to prevent a divide by zero.
        ref->add(std::numeric_limits<Number>::min());
        auto div = (*_residual_vector).clone();
        *div /= *ref;
        ref_resid = Utility::pow<2>(s.calculate_norm(*div, _soln_vars[i], _norm_type));
      }
      else
      {
        ref_resid =
            Utility::pow<2>(s.calculate_norm(*_reference_vector, _soln_vars[i], _norm_type));
        if (_unscale_the_residual)
          ref_resid /= Utility::pow<2>(_scaling_factors[i]);
      }
      _group_ref_resid[group] += ref_resid;
    }
  }

  for (const auto i : index_range(_group_resid))
  {
    _group_resid[i] = std::sqrt(_group_resid[i]);
    _group_ref_resid[i] = std::sqrt(_group_ref_resid[i]);
  }
}

void
ReferenceResidualConvergence::nonlinearConvergenceSetup()
{
  // If no refernce_vector is provided, just revert to DefaultNonlinearConvergence behavior
  if (!_reference_vector)
    return;

  updateReferenceResidual();

  std::ostringstream out;
  out << _name << ": " << _norm_type_enum << " Reference Residual check\n";

  if (_group_names.size() > 0)
  {
    // Set residual and references so that they always have a spacing of 8
    out << std::setprecision(2) << std::scientific;
    unsigned int var_space = 0;
    for (const auto i : index_range(_group_names))
      if (_group_names[i].size() > var_space)
        var_space = _group_names[i].size();

    for (const auto i : index_range(_group_names))
    {
      if (_converge_on_group[i])
      {
        // Print residual
        out << "   " << std::setw(var_space + 8) << std::right << _group_names[i] + "-> res: "
            << (_group_resid[i] < _abs_tol ? COLOR_YELLOW : COLOR_DEFAULT) << std::setw(8)
            << _group_resid[i] << COLOR_DEFAULT;

        // Print res/ref ratio
        if (_local_norm)
          out << "  local res/ref: "
              << (_group_resid[i] / _group_ref_resid[i] < _rel_tol ? COLOR_GREEN : COLOR_DEFAULT)
              << std::setw(8) << _group_ref_resid[i] << COLOR_DEFAULT << "\n";
        else
        {
          // Print reference first if not local norm
          out << "  ref: " << std::setw(8) << _group_ref_resid[i] << "  res/ref: ";

          if (!_group_ref_resid[i])
            out << _group_resid[i] << "\n";
          else
            out << (_group_resid[i] / _group_ref_resid[i] < _rel_tol ? COLOR_GREEN : COLOR_DEFAULT)
                << std::setw(8) << _group_resid[i] / _group_ref_resid[i] << COLOR_DEFAULT << "\n";
        }
      }
    }
    _console << out.str() << std::flush;
  }
}

bool
ReferenceResidualConvergence::checkConvergenceIndividVars(
    const Real /*fnorm*/,
    const Real abs_tol,
    const Real rel_tol,
    const Real /*initial_residual_before_preset_bcs*/)
{
  // Convergence is checked via:
  // 1) Ratio of group residual to reference is less than relative tolerance
  // 2) if group residual is less than absolute tolerance
  // 3) if group reference residual is zero and:
  //   3.1) Convergence type is ZERO_TOLERANCE and group residual is zero (rare, but possible, and
  //        historically implemented that way)
  //   3.2) Convergence type is RELATIVE_TOLERANCE and group residual
  //        is less than relative tolerance. (i.e., using the relative tolerance to check group
  //        convergence in an absolute way)

  bool convergedRelative = true;
  for (const auto i : index_range(_group_resid))
    convergedRelative &=
        ((!_local_norm && _group_resid[i] < _group_ref_resid[i] * rel_tol) ||
         (_local_norm && _group_ref_resid[i] < rel_tol) || _group_resid[i] < abs_tol ||
         (!_group_ref_resid[i] && !_local_norm &&
          ((_zero_ref_type == ZeroReferenceType::ZERO_TOLERANCE && !_group_resid[i]) ||
           (_zero_ref_type == ZeroReferenceType::RELATIVE_TOLERANCE &&
            _group_resid[i] <= rel_tol))));
  return convergedRelative;
}

bool
ReferenceResidualConvergence::checkResidualConvergence(const unsigned int it,
                                                       const Real fnorm,
                                                       const Real ref_norm,
                                                       const Real rel_tol,
                                                       const Real abs_tol,
                                                       std::ostringstream & oss)
{
  // If no refernce_vector is provided, just revert to DefaultNonlinearConvergence behavior
  if (!_reference_vector)
    return DefaultNonlinearConvergence::checkResidualConvergence(
        it, fnorm, ref_norm, rel_tol, abs_tol, oss);

  if (checkConvergenceIndividVars(fnorm, abs_tol, rel_tol, ref_norm))
  {
    oss << "Converged normally";
    return true;
  }
  else if (it >= _accept_iters &&
           checkConvergenceIndividVars(
               fnorm, abs_tol * _accept_mult, rel_tol * _accept_mult, ref_norm))
  {
    oss << "  Converged due a larger acceptable tolerance due to `acceptible_multiplier` after "
           "`acceptible_iterations`.";
    _console << "  Converged due to ACCEPTABLE tolerances" << std::endl;
    return true;
  }

  return false;
}
