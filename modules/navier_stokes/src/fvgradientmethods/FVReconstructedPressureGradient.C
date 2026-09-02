//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVReconstructedPressureGradient.h"

#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "RhieChowMassFlux.h"
#include "SystemBase.h"

#include "libmesh/numeric_vector.h"

registerMooseObject("NavierStokesApp", FVReconstructedPressureGradient);

InputParameters
FVReconstructedPressureGradient::validParams()
{
  InputParameters params = FVGradientMethod::validParams();
  params.addClassDescription("Pressure gradient method that uses Rhie-Chow reconstructed "
                             "gradients once they are available, and otherwise falls back to a "
                             "base gradient method.");
  params.addParam<GradientMethodName>(
      "base_gradient_method",
      "green-gauss",
      "Gradient method used before Rhie-Chow has computed reconstructed gradients.");
  params.addRangeCheckedParam<Real>(
      "gradient_relaxation",
      0.1,
      "0.0<gradient_relaxation<=1.0",
      "Relaxation factor applied when updating reconstructed pressure gradients used by the "
      "momentum predictor.");
  return params;
}

FVReconstructedPressureGradient::FVReconstructedPressureGradient(const InputParameters & params)
  : FVGradientMethod(params),
    _base_gradient_method_name(getParam<GradientMethodName>("base_gradient_method")),
    _gradient_relaxation(getParam<Real>("gradient_relaxation"))
{
}

const FVGradientMethod &
FVReconstructedPressureGradient::resolveBaseGradientMethod(SystemBase & system) const
{
  auto & fe_problem = system.feProblem();
  if (_base_gradient_method_name == name())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' cannot use itself as its base_gradient_method.");

  if (_base_gradient_method_name == "green-gauss" &&
      !fe_problem.hasFVGradientMethod(_base_gradient_method_name))

  {
    auto params = fe_problem.getMooseApp().getFactory().getValidParams("FVGreenGaussGradient");
    fe_problem.addFVGradientMethod("FVGreenGaussGradient", _base_gradient_method_name, params);
  }

  if (!fe_problem.hasFVGradientMethod(_base_gradient_method_name))
    mooseError(
        "Unable to find base FVGradientMethod with name '", _base_gradient_method_name, "'.");

  const auto & method = fe_problem.getFVGradientMethod(_base_gradient_method_name);
  if (&method == this)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' cannot use itself as its base_gradient_method.");

  return method;
}

void
FVReconstructedPressureGradient::setupDependencies(SystemBase & system,
                                                   const unsigned int /*variable_number*/) const
{
  if (!_base_gradient_method)
    _base_gradient_method = &resolveBaseGradientMethod(system);
}

void
FVReconstructedPressureGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & gradient,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  if (!_base_gradient_method)
    mooseError("FVReconstructedPressureGradient '", name(), "' has not been set up.");

  // This method replaces the pressure gradient used by Rhie-Chow. Compatibility between the
  // registered pressure variable and the gradient method is enforced by RhieChowMassFlux.
  if (variable_numbers.empty())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' expects at least one pressure variable number.");

  // When no feedback has been initialized yet or the stored feedback layout no longer matches
  // the requested gradient storage (for example after mesh changes), fall back to the base
  // gradient method.
  const auto feedback_components = _feedback.size();
  bool feedback_layout_matches = _feedback_initialized && feedback_components == gradient.size();

  if (feedback_layout_matches)
  {
    for (const auto component : index_range(gradient))
    {
      const auto & stored = *_feedback[component];
      if (stored.size() != gradient[component]->size() ||
          stored.local_size() != gradient[component]->local_size())
      {
        feedback_layout_matches = false;
        break;
      }
    }
  }

  if (!feedback_layout_matches)
  {
    _base_gradient_method->computeGradient(system, gradient, variable_numbers);
    return;
  }

  // After feedback has been initialized, publish the stored relaxed gradient without further
  // modification so repeated reads are idempotent.
  for (const auto component : index_range(gradient))
    *gradient[component] = *_feedback[component];
}

void
FVReconstructedPressureGradient::updateFeedbackGradient(
    const GradientContainer & base_gradient,
    const GradientContainer & reconstructed_candidate) const
{
  const auto num_components = base_gradient.size();
  if (num_components == 0)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' requires a nonzero number of gradient components when updating the "
               "reconstructed pressure gradient.");

  if (reconstructed_candidate.size() != num_components)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' requires base and reconstructed gradients to have the same number of "
               "components.");

  for (const auto component : index_range(base_gradient))
  {
    const auto & base_vec = *base_gradient[component];
    const auto & candidate_vec = *reconstructed_candidate[component];

    if (base_vec.size() != candidate_vec.size() ||
        base_vec.local_size() != candidate_vec.local_size())
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' requires base and reconstructed gradient components to have the same "
                 "layout.");
  }

  // Whether the currently allocated storage (if any) still matches the base gradient's layout.
  // This is checked independently of _feedback_initialized: resetFeedback() only invalidates the
  // stored *values* for a new time step, it does not deallocate, so storage allocated in a
  // previous time step can be reused here instead of being freed and recreated with clone().
  bool storage_matches = _feedback.size() == num_components;
  if (storage_matches)
  {
    for (const auto component : index_range(_feedback))
    {
      const auto & stored = *_feedback[component];
      const auto & base_vec = *base_gradient[component];
      if (stored.size() != base_vec.size() || stored.local_size() != base_vec.local_size())
      {
        storage_matches = false;
        break;
      }
    }
  }

  // No storage yet, or its layout no longer matches (for example after mesh changes): allocate
  // fresh vectors from the base gradient's layout.
  if (!storage_matches)
  {
    _feedback.clear();
    for (const auto component : index_range(base_gradient))
      _feedback.push_back(base_gradient[component]->clone());
  }

  // Either freshly allocated above, or explicitly reset by resetFeedback() at the start of this
  // time step: (re)seed the values from the base gradient without allocating anything.
  if (!storage_matches || !_feedback_initialized)
  {
    for (const auto component : index_range(_feedback))
    {
      *_feedback[component] = *base_gradient[component];
      _feedback[component]->close();
    }

    _feedback_initialized = true;
    _feedback_generation = 0;
  }

  const Real alpha = _gradient_relaxation;

  for (const auto component : index_range(_feedback))
  {
    auto & stored_gradient = *_feedback[component];
    stored_gradient.scale(1.0 - alpha);
    stored_gradient.add(alpha, *reconstructed_candidate[component]);
    stored_gradient.close();
  }

  ++_feedback_generation;
}

void
FVReconstructedPressureGradient::resetFeedback() const
{
  // Deliberately do not clear _feedback: the vector layout is still valid for the next time
  // step, so keeping it allocated lets updateFeedbackGradient() reuse this storage instead of
  // deallocating and reallocating (via clone()) on every time step.
  _feedback_initialized = false;
  _feedback_generation = 0;
}
