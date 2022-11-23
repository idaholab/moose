//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedReturnMappingSolution.h"

#include "Moose.h"
#include "MooseEnum.h"
#include "MooseObject.h"
#include "ConsoleStreamInterface.h"
#include "Conversion.h"
#include "MathUtils.h"

#include "DualRealOps.h"

#include <limits>
#include <string>
#include <cmath>
#include <memory>

template <bool is_ad>
InputParameters
GeneralizedReturnMappingSolutionTempl<is_ad>::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>(
      "relative_tolerance", 1e-8, "Relative convergence tolerance for Newton iteration");
  params.addParam<Real>(
      "absolute_tolerance", 1e-11, "Absolute convergence tolerance for Newton iteration");
  params.addParam<Real>("acceptable_multiplier",
                        10,
                        "Factor applied to relative and absolute "
                        "tolerance for acceptable convergence if "
                        "iterations are no longer making progress");

  // diagnostic output parameters
  MooseEnum internal_solve_output_on_enum("never on_error always", "on_error");
  params.addParam<MooseEnum>("internal_solve_output_on",
                             internal_solve_output_on_enum,
                             "When to output internal Newton solve information");
  params.addParam<bool>("internal_solve_full_iteration_history",
                        false,
                        "Set true to output full internal Newton iteration history at times "
                        "determined by `internal_solve_output_on`. If false, only a summary is "
                        "output.");
  params.addParamNamesToGroup("internal_solve_output_on internal_solve_full_iteration_history",
                              "Debug");
  return params;
}
template <bool is_ad>
GeneralizedReturnMappingSolutionTempl<is_ad>::GeneralizedReturnMappingSolutionTempl(
    const InputParameters & parameters)
  : _check_range(false),
    _line_search(true),
    _bracket_solution(false),
    _internal_solve_output_on(
        parameters.get<MooseEnum>("internal_solve_output_on").getEnum<InternalSolveOutput>()),
    _max_its(50),
    _internal_solve_full_iteration_history(
        parameters.get<bool>("internal_solve_full_iteration_history")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _acceptable_multiplier(parameters.get<Real>("acceptable_multiplier")),
    _num_resids(30),
    _residual_history(_num_resids, std::numeric_limits<Real>::max()),
    _iteration(0),
    _initial_residual(0.0),
    _residual(0.0),
    _svrms_name(parameters.get<std::string>("_object_name"))
{
}

template <bool is_ad>
GenericReal<is_ad>
GeneralizedReturnMappingSolutionTempl<is_ad>::minimumPermissibleValue(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/) const
{
  return std::numeric_limits<Real>::lowest();
}

template <bool is_ad>
GenericReal<is_ad>
GeneralizedReturnMappingSolutionTempl<is_ad>::maximumPermissibleValue(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/) const
{
  return std::numeric_limits<Real>::max();
}

template <bool is_ad>
void
GeneralizedReturnMappingSolutionTempl<is_ad>::returnMappingSolve(
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericDenseVector<is_ad> & stress_new,
    GenericReal<is_ad> & scalar,
    const ConsoleStream & console)
{
  // construct the stringstream here only if the debug level is set to ALL
  std::unique_ptr<std::stringstream> iter_output =
      (_internal_solve_output_on == InternalSolveOutput::ALWAYS)
          ? std::make_unique<std::stringstream>()
          : nullptr;

  // do the internal solve and capture iteration info during the first round
  // iff full history output is requested regardless of whether the solve failed or succeeded
  auto solve_state =
      internalSolve(stress_dev,
                    stress_new,
                    scalar,
                    _internal_solve_full_iteration_history ? iter_output.get() : nullptr);
  if (solve_state != SolveState::SUCCESS &&
      _internal_solve_output_on != InternalSolveOutput::ALWAYS)
  {
    // output suppressed by user, throw immediately
    if (_internal_solve_output_on == InternalSolveOutput::NEVER)
      mooseException("");

    // user expects some kind of output, if necessary setup output stream now
    if (!iter_output)
      iter_output = std::make_unique<std::stringstream>();

    // add the appropriate error message to the output
    switch (solve_state)
    {
      case SolveState::NAN_INF:
        *iter_output << "Encountered inf or nan in material return mapping iterations.\n";
        break;

      case SolveState::EXCEEDED_ITERATIONS:
        *iter_output << "Exceeded maximum iterations in material return mapping iterations.\n";
        break;

      default:
        mooseError("Unhandled solver state");
    }

    // if full history output is only requested for failed solves we have to repeat
    // the solve a second time
    if (_internal_solve_full_iteration_history)
      internalSolve(stress_dev, stress_new, scalar, iter_output.get());

    // Append summary and throw exception
    outputIterationSummary(iter_output.get(), _iteration);
    mooseException(iter_output->str());
  }

  if (_internal_solve_output_on == InternalSolveOutput::ALWAYS)
  {
    // the solve did not fail but the user requested debug output anyways
    outputIterationSummary(iter_output.get(), _iteration);
    console << iter_output->str();
  }
}

template <bool is_ad>
typename GeneralizedReturnMappingSolutionTempl<is_ad>::SolveState
GeneralizedReturnMappingSolutionTempl<is_ad>::internalSolve(
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericDenseVector<is_ad> & stress_new,
    GenericReal<is_ad> & delta_gamma,
    std::stringstream * iter_output)
{
  delta_gamma = initialGuess(stress_dev);
  GenericReal<is_ad> scalar_old = delta_gamma;
  GenericReal<is_ad> scalar_increment = 0.0;
  const GenericReal<is_ad> min_permissible_scalar = minimumPermissibleValue(stress_dev);
  const GenericReal<is_ad> max_permissible_scalar = maximumPermissibleValue(stress_dev);
  GenericReal<is_ad> scalar_upper_bound = max_permissible_scalar;
  GenericReal<is_ad> scalar_lower_bound = min_permissible_scalar;
  _iteration = 0;

  _initial_residual = _residual = computeResidual(stress_dev, stress_new, delta_gamma);

  Real init_resid_sign = MathUtils::sign(MetaPhysicL::raw_value(_residual));
  Real reference_residual =
      computeReferenceResidual(stress_dev, stress_new, _residual, delta_gamma);

  if (converged(_residual, reference_residual))
  {
    iterationFinalize(delta_gamma);
    outputIterationStep(iter_output, stress_dev, delta_gamma, reference_residual);
    return SolveState::SUCCESS;
  }

  _residual_history.assign(_num_resids, std::numeric_limits<Real>::max());
  _residual_history[0] = MetaPhysicL::raw_value(_residual);

  while (_iteration < _max_its && !converged(_residual, reference_residual) &&
         !convergedAcceptable(_iteration, reference_residual))
  {
    scalar_increment = -_residual / computeDerivative(stress_dev, stress_new, delta_gamma);
    delta_gamma = scalar_old + scalar_increment;

    if (_check_range)
      checkPermissibleRange(delta_gamma,
                            scalar_increment,
                            scalar_old,
                            min_permissible_scalar,
                            max_permissible_scalar,
                            iter_output);

    _residual = computeResidual(stress_dev, stress_new, delta_gamma);

    reference_residual = computeReferenceResidual(stress_dev, stress_new, _residual, delta_gamma);
    iterationFinalize(delta_gamma);

    if (_bracket_solution)
      updateBounds(delta_gamma,
                   _residual,
                   init_resid_sign,
                   scalar_upper_bound,
                   scalar_lower_bound,
                   iter_output);

    if (converged(_residual, reference_residual))
    {
      outputIterationStep(iter_output, stress_dev, delta_gamma, reference_residual);
      break;
    }
    else
    {
      bool modified_increment = false;

      if (_bracket_solution)
      {
        // Check to see whether trial scalar_increment is outside the bounds, and set it to a point
        // within the bounds if it is
        if (scalar_old + scalar_increment >= scalar_upper_bound ||
            scalar_old + scalar_increment <= scalar_lower_bound)
        {
          if (scalar_upper_bound != max_permissible_scalar &&
              scalar_lower_bound != min_permissible_scalar)
          {
            const Real frac = 0.5;
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
        delta_gamma = scalar_old + scalar_increment;
        _residual = computeResidual(stress_dev, stress_new, delta_gamma);
        reference_residual =
            computeReferenceResidual(stress_dev, stress_new, _residual, delta_gamma);
        iterationFinalize(delta_gamma);

        if (_bracket_solution)
          updateBounds(delta_gamma,
                       _residual,
                       init_resid_sign,
                       scalar_upper_bound,
                       scalar_lower_bound,
                       iter_output);
      }
    }

    outputIterationStep(iter_output, stress_dev, delta_gamma, reference_residual);

    ++_iteration;
    scalar_old = delta_gamma;
    _residual_history[_iteration % _num_resids] = MetaPhysicL::raw_value(_residual);
  }

  if (std::isnan(_residual) || std::isinf(MetaPhysicL::raw_value(_residual)))
    return SolveState::NAN_INF;

  if (_iteration == _max_its)
    return SolveState::EXCEEDED_ITERATIONS;

  return SolveState::SUCCESS;
}

template <bool is_ad>
bool
GeneralizedReturnMappingSolutionTempl<is_ad>::converged(const GenericReal<is_ad> & ad_residual,
                                                        const Real & reference)
{
  const Real residual = MetaPhysicL::raw_value(ad_residual);
  return (std::abs(residual) <= _absolute_tolerance ||
          std::abs(residual / reference) <= _relative_tolerance);
}

template <bool is_ad>
bool
GeneralizedReturnMappingSolutionTempl<is_ad>::convergedAcceptable(const unsigned int it,
                                                                  const Real & reference)
{
  // Require that we have at least done _num_resids evaluations before we allow for
  // acceptable convergence
  if (it < _num_resids)
    return false;

  // Check to see whether the residual has dropped by convergence_history_factor over
  // the last _num_resids iterations. If it has (which means it's still making progress),
  // don't consider it to be converged within the acceptable limits.
  const Real convergence_history_factor = 10.0;
  if (std::abs(_residual * convergence_history_factor) <
      std::abs(_residual_history[(it + 1) % _num_resids]))
    return false;

  // Now that it's determined that progress is not being made, treat it as converged if
  // we're within the acceptable convergence limits
  return converged(_residual / _acceptable_multiplier, reference);
}

template <bool is_ad>
void
GeneralizedReturnMappingSolutionTempl<is_ad>::checkPermissibleRange(
    GenericReal<is_ad> & scalar,
    GenericReal<is_ad> & scalar_increment,
    const GenericReal<is_ad> & scalar_old,
    const GenericReal<is_ad> min_permissible_scalar,
    const GenericReal<is_ad> max_permissible_scalar,
    std::stringstream * iter_output)
{
  if (scalar > max_permissible_scalar)
  {
    scalar_increment = (max_permissible_scalar - scalar_old) / 2.0;
    scalar = scalar_old + scalar_increment;
    if (iter_output)
      *iter_output << "Scalar greater than maximum ("
                   << MetaPhysicL::raw_value(max_permissible_scalar)
                   << ") adjusted scalar=" << MetaPhysicL::raw_value(scalar)
                   << " scalar_increment=" << MetaPhysicL::raw_value(scalar_increment) << std::endl;
  }
  else if (scalar < min_permissible_scalar)
  {
    scalar_increment = (min_permissible_scalar - scalar_old) / 2.0;
    scalar = scalar_old + scalar_increment;
    if (iter_output)
      *iter_output << "Scalar less than minimum (" << MetaPhysicL::raw_value(min_permissible_scalar)
                   << ") adjusted scalar=" << MetaPhysicL::raw_value(scalar)
                   << " scalar_increment=" << MetaPhysicL::raw_value(scalar_increment) << std::endl;
  }
}

template <bool is_ad>
void
GeneralizedReturnMappingSolutionTempl<is_ad>::updateBounds(const GenericReal<is_ad> & scalar,
                                                           const GenericReal<is_ad> & residual,
                                                           const Real init_resid_sign,
                                                           GenericReal<is_ad> & scalar_upper_bound,
                                                           GenericReal<is_ad> & scalar_lower_bound,
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
}

template <bool is_ad>
void
GeneralizedReturnMappingSolutionTempl<is_ad>::outputIterationStep(
    std::stringstream * iter_output,
    const GenericDenseVector<is_ad> & effective_trial_stress,
    const GenericReal<is_ad> & scalar,
    const GenericReal<is_ad> reference_residual)
{
  if (iter_output)
  {
    const unsigned int it = _iteration;
    const Real residual = MetaPhysicL::raw_value(_residual);

    *iter_output << " iteration=" << it
                 << " trial_stress=" << MetaPhysicL::raw_value(effective_trial_stress)
                 << " scalar=" << MetaPhysicL::raw_value(scalar) << " residual=" << residual
                 << " ref_res=" << reference_residual
                 << " rel_res=" << std::abs(residual) / reference_residual
                 << " rel_tol=" << _relative_tolerance << " abs_res=" << std::abs(residual)
                 << " abs_tol=" << _absolute_tolerance << '\n';
  }
}

template <bool is_ad>
void
GeneralizedReturnMappingSolutionTempl<is_ad>::outputIterationSummary(
    std::stringstream * iter_output, const unsigned int total_it)
{
  if (iter_output)
    *iter_output << "In " << total_it << " iterations the residual went from "
                 << MetaPhysicL::raw_value(_initial_residual) << " to "
                 << MetaPhysicL::raw_value(_residual) << " in '" << _svrms_name << "'.\n";
}

template class GeneralizedReturnMappingSolutionTempl<false>;
template class GeneralizedReturnMappingSolutionTempl<true>;
