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
    _reference_vector(nullptr),
    _converge_on(getParam<std::vector<NonlinearVariableName>>("converge_on")),
    _zero_ref_type(
        getParam<MooseEnum>("zero_reference_residual_treatment").getEnum<ZeroReferenceType>()),
    _reference_vector_tag_id(Moose::INVALID_TAG_ID),
    _initialized(false)
{
  if (parameters.isParamValid("solution_variables"))
  {
    if (parameters.isParamValid("reference_vector"))
      mooseDeprecated("The `solution_variables` parameter is deprecated, has no effect when "
                      "the tagging system is used, and will be removed on January 1, 2020. "
                      "Please simply delete this parameter from your input file.");
    _soln_var_names = parameters.get<std::vector<NonlinearVariableName>>("solution_variables");
  }

  if (parameters.isParamValid("reference_residual_variables") &&
      parameters.isParamValid("reference_vector"))
    mooseError(
        "You may specify either the `reference_residual_variables` "
        "or `reference_vector` parameter, not both. `reference_residual_variables` is deprecated "
        "so we recommend using `reference_vector`");

  if (parameters.isParamValid("reference_residual_variables"))
  {
    mooseDeprecated(
        "The save-in method for composing reference residual quantities is deprecated "
        "and will be removed on January 1, 2020. Please use the tagging system instead; "
        "specifically, please assign a TagName to the `reference_vector` parameter");

    _ref_resid_var_names =
        parameters.get<std::vector<AuxVariableName>>("reference_residual_variables");

    if (_soln_var_names.size() != _ref_resid_var_names.size())
      mooseError("Size of solution_variables (",
                 _soln_var_names.size(),
                 ") != size of reference_residual_variables (",
                 _ref_resid_var_names.size(),
                 ")");
  }
  else if (parameters.isParamValid("reference_vector"))
  {
    if (_fe_problem.numNonlinearSystems() > 1)
      paramError(
          "nl_sys_names",
          "reference residual problem does not currently support multiple nonlinear systems");
    _reference_vector_tag_id = _fe_problem.getVectorTagID(getParam<TagName>("reference_vector"));
    _reference_vector = &_fe_problem.getNonlinearSystemBase(0).getVector(_reference_vector_tag_id);
  }
  else
    mooseInfo("Neither the `reference_residual_variables` nor `reference_vector` parameter is "
              "specified, which means that no reference "
              "quantites are set. Because of this, the standard technique of comparing the "
              "norm of the full residual vector with its initial value will be used.");

  _accept_mult = parameters.get<Real>("acceptable_multiplier");
  _accept_iters = parameters.get<unsigned int>("acceptable_iterations");

  const auto norm_type_enum =
      parameters.get<MooseEnum>("normalization_type").getEnum<NormalizationType>();
  if (norm_type_enum == NormalizationType::LOCAL_L2)
  {
    _norm_type = libMesh::DISCRETE_L2;
    _local_norm = true;
  }
  else if (norm_type_enum == NormalizationType::GLOBAL_L2)
  {
    _norm_type = libMesh::DISCRETE_L2;
    _local_norm = false;
  }
  else if (norm_type_enum == NormalizationType::LOCAL_LINF)
  {
    _norm_type = libMesh::DISCRETE_L_INF;
    _local_norm = true;
  }
  else if (norm_type_enum == NormalizationType::GLOBAL_LINF)
  {
    _norm_type = libMesh::DISCRETE_L_INF;
    _local_norm = false;
  }
  else
  {
    mooseAssert(false, "This point should not be reached.");
  }

  if (_local_norm && !parameters.isParamValid("reference_vector"))
    paramError("reference_vector", "If local norm is used, a reference_vector must be provided.");
}

void
ReferenceResidualConvergence::initialSetup()
{
  DefaultNonlinearConvergence::initialSetup();

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
    mooseError("Size of solution_variables (",
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
  // If not using groups, use one group for each variable
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
    bool found_match = false;
    for (unsigned int var_num = 0; var_num < s.n_vars(); var_num++)
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

  for (const auto i : index_range(_soln_vars))
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
    // Check for variable groups containing both field and scalar variables
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

  // Keep track of the names of the variables in each group (of 1 variable if not using groups)
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
    const unsigned int size_soln_vars = _soln_vars.size();
    _scaling_factors.resize(size_soln_vars);
    for (unsigned int i = 0; i < size_soln_vars; ++i)
      if (nonlinear_sys.isScalarVariable(_soln_vars[i]))
        _scaling_factors[i] = nonlinear_sys.getScalarVariable(0, _soln_vars[i]).scalingFactor();
      else
        _scaling_factors[i] = nonlinear_sys.getVariable(/*tid*/ 0, _soln_vars[i]).scalingFactor();
  }
  _initialized = true;
}

void
ReferenceResidualConvergence::updateReferenceResidual()
{
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

  for (const auto i : index_range(_soln_vars))
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
  if (!_initialized)
    initialSetup();

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
  //        historically implemented that way)
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
ReferenceResidualConvergence::checkRelativeConvergence(const unsigned int it,
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
