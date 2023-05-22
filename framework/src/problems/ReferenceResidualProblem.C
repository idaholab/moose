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
  MooseEnum Lnorm("global_L2 local_L2 global_Linf local_Linf", "global_L2");
  params.addParam<MooseEnum>(
      "normalization_type",
      Lnorm,
      "The normalization type used to compare the reference and actual residuals.");
  Lnorm.addDocumentation("global_L2",
                         "Compare the L2 norm of the residual vector to the L2 norm of the "
                         "absolute reference vector to determine relative convergence");
  Lnorm.addDocumentation(
      "local_L2",
      "Compute the L2 norm of the residual vector divided componentwise by the absolute reference "
      "vector to the L2 norm of the absolute reference vector to determine relative convergence");
  Lnorm.addDocumentation(
      "global_Linf",
      "Compare the L-infinity norm of the residual vector to the L-infinity norm of the "
      "absolute reference vector to determine relative convergence");
  Lnorm.addDocumentation("local_Linf",
                         "Compute the L-infinity norm of the residual vector divided componentwise "
                         "by the absolute reference "
                         "vector to the L-infinity norm of the absolute reference vector to "
                         "determine relative convergence");

  MooseEnum zero_ref_res("zero_tolerance relative_tolerance", "relative_tolerance");
  params.addParam<MooseEnum>("zero_reference_residual_treatment",
                             zero_ref_res,
                             "Determine behavior if a reference residual value of zero is present "
                             "for a particular variable.");
  zero_ref_res.addDocumentation("zero_tolerance",
                                "Solve is treated as converged if the residual is zero");
  zero_ref_res.addDocumentation(
      "relative_tolerance",
      "Solve is treated as converged if the residual is below the relative tolerance");
  return params;
}

ReferenceResidualProblem::ReferenceResidualProblem(const InputParameters & params)
  : FEProblem(params),
    _use_group_variables(false),
    _reference_vector(nullptr),
    _converge_on(getParam<std::vector<NonlinearVariableName>>("converge_on")),
    _zero_ref_type(
        params.get<MooseEnum>("zero_reference_residual_treatment").getEnum<ZeroReferenceType>()),
    _reference_vector_tag_id(Moose::INVALID_TAG_ID)
{
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
    _reference_vector_tag_id = getVectorTagID(getParam<TagName>("reference_vector"));
    _reference_vector = &getNonlinearSystemBase(0).getVector(_reference_vector_tag_id);
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

  const auto norm_type_enum =
      params.get<MooseEnum>("normalization_type").getEnum<NormalizationType>();
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

  if (_local_norm && !params.isParamValid("reference_vector"))
    paramError("reference_vector", "If local norm is used, a reference_vector must be provided.");
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
      mooseAssert(nonlinear_sys.RHS().size() == (*_reference_vector).size(),
                  "Sizes of nonlinear RHS and reference vector should be the same.");
      mooseAssert((*_reference_vector).size(), "Reference vector must be provided.");
      // Add a tiny number to the reference to prevent a divide by zero.
      auto ref = _reference_vector->clone();
      ref->add(std::numeric_limits<Number>::min());
      auto div = nonlinear_sys.RHS().clone();
      *div /= *ref;
      resid = Utility::pow<2>(s.calculate_norm(*div, _soln_vars[i], _norm_type));
    }
    else
    {
      resid = Utility::pow<2>(s.calculate_norm(nonlinear_sys.RHS(), _soln_vars[i], _norm_type));
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
ReferenceResidualProblem::nonlinearConvergenceSetup()
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
ReferenceResidualProblem::checkRelativeConvergence(const PetscInt it,
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

bool
ReferenceResidualProblem::checkConvergenceIndividVars(const Real fnorm,
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
