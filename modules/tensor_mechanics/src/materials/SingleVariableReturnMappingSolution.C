/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SingleVariableReturnMappingSolution.h"

#include "InputParameters.h"
#include <cmath>
#include "Conversion.h"

template <>
InputParameters
validParams<SingleVariableReturnMappingSolution>()
{
  InputParameters params = emptyInputParameters();

  // Newton iteration control parameters
  params.addParam<unsigned int>("max_its", 300, "Maximum number of Newton iterations");
  params.addParam<unsigned int>(
      "maximum_iterations",
      300,
      "Maximum number of Newton iterations with old tensor mechanics parameter");
  params.addParam<bool>(
      "output_iteration_info", false, "Set true to output Newton iteration information");
  params.addDeprecatedParam<bool>(
      "output_iteration_info_on_error",
      false,
      "Set true to output Newton iteration information when those iterations fail",
      "This information is always output when iterations fail");
  params.addParam<Real>(
      "relative_tolerance", 1e-8, "Relative convergence tolerance for Newton iteration");
  params.addParam<Real>(
      "absolute_tolerance", 1e-11, "Absolute convergence tolerance for Newton iteration");
  params.addParam<bool>("legacy_return_mapping",
                        false,
                        "Perform iterations and compute residual "
                        "the same way the same way as the previous "
                        "algorithm. Also use same old defaults for relative_tolerance, "
                        "absolute_tolerance, and max_its.");

  return params;
}

SingleVariableReturnMappingSolution::SingleVariableReturnMappingSolution(
    const InputParameters & parameters)
  : _legacy_return_mapping(false),
    _check_range(false),
    _max_its(parameters.isParamSetByUser("max_iterations")
                 ? parameters.get<unsigned int>("max_iterations")
                 : parameters.get<unsigned int>("max_its")),
    _output_iteration_info(parameters.get<bool>("output_iteration_info")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _line_search(true),
    _bracket_solution(true)
{
  if (parameters.get<bool>("legacy_return_mapping") == true)
  {
    if (!parameters.isParamSetByUser("relative_tolerance"))
      _relative_tolerance = 1.e-5;
    if (!parameters.isParamSetByUser("absolute_tolerance"))
      _absolute_tolerance = 1.e-20;
    if (!parameters.isParamSetByUser("max_its"))
      _max_its = 30;
    _line_search = false;
    _bracket_solution = false;
    _check_range = false;
    _legacy_return_mapping = true;
  }
}

Real
SingleVariableReturnMappingSolution::maximumPermissibleValue(
    const Real /*effective_trial_stress*/) const
{
  return std::numeric_limits<Real>::max();
}

void
SingleVariableReturnMappingSolution::returnMappingSolve(const Real effective_trial_stress,
                                                        Real & scalar,
                                                        const ConsoleStream & console)
{
  std::stringstream iter_output;
  std::stringstream * iter_output_ptr = (_output_iteration_info ? &iter_output : nullptr);

  if (!_legacy_return_mapping)
  {
    if (!internalSolve(effective_trial_stress, scalar, iter_output_ptr))
    {
      if (iter_output_ptr)
        mooseError(iter_output_ptr->str());
      else
      {
        internalSolve(effective_trial_stress, scalar, &iter_output);
        mooseError(iter_output.str());
      }
    }
    else if (iter_output_ptr)
      console << iter_output_ptr->str();
  }
  else
  {
    if (!internalSolveLegacy(effective_trial_stress, scalar, iter_output_ptr))
    {
      if (iter_output_ptr)
        mooseError(iter_output_ptr->str());
      else
      {
        internalSolveLegacy(effective_trial_stress, scalar, &iter_output);
        mooseError(iter_output.str());
      }
    }
    else if (iter_output_ptr)
      console << iter_output_ptr->str();
  }
}

bool
SingleVariableReturnMappingSolution::internalSolve(const Real effective_trial_stress,
                                                   Real & scalar,
                                                   std::stringstream * iter_output)
{
  scalar = 0.0;
  Real scalar_old = 0.0;
  Real scalar_increment = 0.0;
  const Real min_permissible_scalar = 0.0;
  const Real max_permissible_scalar = maximumPermissibleValue(effective_trial_stress);
  Real scalar_upper_bound = max_permissible_scalar;
  Real scalar_lower_bound = 0.0;
  unsigned int it = 0;

  if (effective_trial_stress == 0.0)
  {
    outputIterInfo(iter_output, it, effective_trial_stress, scalar, 0.0, 1.0);
    return true;
  }

  Real residual = computeResidual(effective_trial_stress, scalar);
  Real residual_old = residual;

  Real init_resid_norm = std::abs(residual);
  if (init_resid_norm == 0.0)
    init_resid_norm = 1.0;
  Real init_resid_sign = (residual < 0.0 ? -1.0 : 1.0);

  Real reference_residual = computeReferenceResidual(effective_trial_stress, scalar);

  if (converged(residual, reference_residual))
  {
    iterationFinalize(scalar);
    outputIterInfo(iter_output, it, effective_trial_stress, scalar, residual, reference_residual);
    return true;
  }

  while (it < _max_its && !converged(residual, reference_residual))
  {
    scalar_increment = -residual / computeDerivative(effective_trial_stress, scalar);
    scalar = scalar_old + scalar_increment;

    if (_check_range)
      checkPermissibleRange(scalar,
                            scalar_increment,
                            scalar_old,
                            min_permissible_scalar,
                            max_permissible_scalar,
                            iter_output);

    residual = computeResidual(effective_trial_stress, scalar);
    reference_residual = computeReferenceResidual(effective_trial_stress, scalar);
    iterationFinalize(scalar);

    if (_bracket_solution)
      updateBounds(scalar,
                   residual,
                   init_resid_sign,
                   scalar_upper_bound,
                   scalar_lower_bound,
                   max_permissible_scalar,
                   iter_output);

    if (converged(residual, reference_residual))
    {
      outputIterInfo(iter_output, it, effective_trial_stress, scalar, residual, reference_residual);
      break;
    }
    else
    {
      bool modified_increment = false;

      // Line Search
      if (_line_search)
      {
        if (residual_old - residual != 0.0)
        {
          Real alpha = residual_old / (residual_old - residual);
          if (alpha > 1.0) // upper bound for alpha
            alpha = 1.0;
          else if (alpha < 1e-2) // lower bound for alpha
            alpha = 1e-2;
          if (alpha != 1.0)
          {
            modified_increment = true;
            scalar_increment *= alpha;
            if (iter_output)
              *iter_output << "  Line search alpha = " << alpha
                           << " increment = " << scalar_increment << std::endl;
          }
        }
      }

      if (_bracket_solution)
      {
        // Check to see whether trial scalar_increment is outside the bounds, and set it to a point
        // within the bounds if it is
        if (scalar_old + scalar_increment >= scalar_upper_bound ||
            scalar_old + scalar_increment <= scalar_lower_bound)
        {
          if (scalar_upper_bound != max_permissible_scalar && scalar_lower_bound != 0.0)
          {
            Real frac = 0.5;
            scalar_increment =
                (1.0 - frac) * scalar_lower_bound + frac * scalar_upper_bound - scalar_old;
            modified_increment = true;
            if (iter_output)
              *iter_output << "  Trial scalar_increment exceeded bounds.  Setting between "
                              "lower/upper bounds. frac: "
                           << frac << std::endl;
          }
        }
      }

      // Update the trial scalar and recompute residual if the line search or bounds checking
      // modified the increment
      if (modified_increment)
      {
        scalar = scalar_old + scalar_increment;
        residual = computeResidual(effective_trial_stress, scalar);
        reference_residual = computeReferenceResidual(effective_trial_stress, scalar);
        iterationFinalize(scalar);

        if (_bracket_solution)
          updateBounds(scalar,
                       residual,
                       init_resid_sign,
                       scalar_upper_bound,
                       scalar_lower_bound,
                       max_permissible_scalar,
                       iter_output);
      }
    }

    outputIterInfo(iter_output, it, effective_trial_stress, scalar, residual, reference_residual);

    ++it;
    residual_old = residual;
    scalar_old = scalar;
  }

  bool has_converged = true;
  if (std::isnan(residual) || std::isinf(residual))
  {
    has_converged = false;
    if (iter_output)
      *iter_output << "Encountered inf or nan in material return mapping iterations." << std::endl;
  }

  if (it == _max_its)
  {
    has_converged = false;
    if (iter_output)
      *iter_output << "Exceeded maximum iterations in material return mapping iterations."
                   << std::endl;
    ;
  }

  return has_converged;
}

bool
SingleVariableReturnMappingSolution::internalSolveLegacy(const Real effective_trial_stress,
                                                         Real & scalar,
                                                         std::stringstream * iter_output)
{
  scalar = 0.0;
  unsigned int it = 0;
  Real residual = 10.0;
  Real norm_residual = 10.0;
  Real first_norm_residual = 10.0;
  std::string iter_str;

  while (it < _max_its && norm_residual > _absolute_tolerance &&
         (norm_residual / first_norm_residual) > _relative_tolerance)
  {
    residual = computeResidual(effective_trial_stress, scalar);
    norm_residual = std::abs(residual);
    if (it == 0)
    {
      first_norm_residual = norm_residual;
      if (first_norm_residual == 0)
        first_norm_residual = 1;
    }

    scalar -= residual / computeDerivative(effective_trial_stress, scalar);

    if (iter_output)
    {
      iter_str = "Return mapping solve:\n iteration = " + Moose::stringify(it) + "\n" +
                 +" effective trial stress = " + Moose::stringify(effective_trial_stress) + "\n" +
                 +" scalar effective inelastic strain = " + Moose::stringify(scalar) + "\n" +
                 +" relative residual = " + Moose::stringify(norm_residual / first_norm_residual) +
                 "\n" + +" relative tolerance = " + Moose::stringify(_relative_tolerance) + "\n" +
                 +" absolute residual = " + Moose::stringify(norm_residual) + "\n" +
                 +" absolute tolerance = " + Moose::stringify(_absolute_tolerance) + "\n";
    }
    iterationFinalize(scalar);
    ++it;
  }

  if (iter_output)
    *iter_output << iter_str;

  bool has_converged = true;

  if (it == _max_its && norm_residual > _absolute_tolerance &&
      (norm_residual / first_norm_residual) > _relative_tolerance)
  {
    has_converged = false;
    if (iter_output)
      *iter_output << "Exceeded maximum iterations in material return mapping iterations."
                   << std::endl;
  }
  return has_converged;
}

bool
SingleVariableReturnMappingSolution::converged(const Real & residual, const Real & reference)
{
  return (std::abs(residual) <= _absolute_tolerance ||
          (std::abs(residual) / reference) <= _relative_tolerance);
}

void
SingleVariableReturnMappingSolution::outputIterInfo(std::stringstream * iter_output,
                                                    const unsigned int & it,
                                                    const Real & effective_trial_stress,
                                                    const Real & scalar,
                                                    const Real & residual,
                                                    const Real & reference_residual)
{
  if (iter_output)
  {
    *iter_output << " iteration=" << it << " trial_stress=" << effective_trial_stress
                 << " scalar=" << scalar << " residual=" << residual
                 << " ref_res=" << reference_residual
                 << " rel_res=" << std::abs(residual) / reference_residual
                 << " rel_tol=" << _relative_tolerance << " abs_res=" << std::abs(residual)
                 << " abs_tol=" << _absolute_tolerance << std::endl;
  }
}

void
SingleVariableReturnMappingSolution::checkPermissibleRange(Real & scalar,
                                                           Real & scalar_increment,
                                                           const Real scalar_old,
                                                           const Real min_permissible_scalar,
                                                           const Real max_permissible_scalar,
                                                           std::stringstream * iter_output)
{
  if (scalar > max_permissible_scalar)
  {
    scalar_increment = (max_permissible_scalar - scalar_old) / 2.0;
    scalar = scalar_old + scalar_increment;
    if (iter_output)
      *iter_output << "Scalar greater than maximum (" << max_permissible_scalar
                   << ") adjusted scalar=" << scalar << " scalar_increment=" << scalar_increment
                   << std::endl;
  }
  else if (scalar < min_permissible_scalar)
  {
    scalar_increment = (min_permissible_scalar - scalar_old) / 2.0;
    scalar = scalar_old + scalar_increment;
    if (iter_output)
      *iter_output << "Scalar less than minimum (" << min_permissible_scalar
                   << ") adjusted scalar=" << scalar << " scalar_increment=" << scalar_increment
                   << std::endl;
  }
}

void
SingleVariableReturnMappingSolution::updateBounds(const Real & scalar,
                                                  const Real & residual,
                                                  const Real & init_resid_sign,
                                                  Real & scalar_upper_bound,
                                                  Real & scalar_lower_bound,
                                                  const Real & max_permissible_value,
                                                  std::stringstream * iter_output)
{
  // Update upper/lower bounds as applicable
  if (residual * init_resid_sign < 0.0 && scalar < scalar_upper_bound)
  {
    scalar_upper_bound = scalar;
    if (scalar_upper_bound < scalar_lower_bound)
    {
      scalar_upper_bound = scalar_lower_bound;
      scalar_lower_bound = 0.0;
      if (iter_output)
        *iter_output << "  Corrected for scalar_upper_bound < scalar_lower_bound" << std::endl;
    }
  }
  // Don't permit setting scalar_lower_bound > scalar_upper_bound (but do permit the reverse).
  // This ensures that if we encounter multiple roots, we pick the lowest one.
  else if (residual * init_resid_sign > 0.0 && scalar > scalar_lower_bound &&
           scalar < scalar_upper_bound)
    scalar_lower_bound = scalar;

  if ((scalar_lower_bound > 0 &&
       (scalar_upper_bound - scalar_lower_bound) / (PETSC_MACHINE_EPSILON * scalar_lower_bound) <=
           10.0))
  {
    // Reset lower/upper bounds.  They're too close together and solution hasn't converged, so the
    // solution is likely outside those bounds
    scalar_lower_bound = 0.0;
    scalar_upper_bound = max_permissible_value;
    if (iter_output)
      *iter_output << "  Reset lower/upper bounds because they're too close together" << std::endl;
  }
}
