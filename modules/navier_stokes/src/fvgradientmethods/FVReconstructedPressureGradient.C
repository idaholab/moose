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
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object", "Rhie-Chow user object that owns the reconstructed gradients.");
  params.addParam<GradientMethodName>(
      "base_gradient_method",
      "green-gauss",
      "Gradient method used before Rhie-Chow has computed reconstructed gradients.");
  return params;
}

FVReconstructedPressureGradient::FVReconstructedPressureGradient(const InputParameters & params)
  : FVGradientMethod(params),
    _rhie_chow_user_object_name(getParam<UserObjectName>("rhie_chow_user_object")),
    _base_gradient_method_name(getParam<GradientMethodName>("base_gradient_method"))
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
    fe_problem.addFVGradientMethod("FVGreenGaussGradient", _base_gradient_method_name);

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
                                                   const unsigned int variable_number) const
{
  if (!_base_gradient_method)
    _base_gradient_method = &resolveBaseGradientMethod(system);

  if (!_rhie_chow_user_object)
    _rhie_chow_user_object =
        &system.feProblem().getUserObject<RhieChowMassFlux>(_rhie_chow_user_object_name);

  if (variable_number != _rhie_chow_user_object->pressureVariableNumber())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' can only be used for the pressure variable registered on RhieChowMassFlux '",
               _rhie_chow_user_object_name,
               "'.");
}

void
FVReconstructedPressureGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & output_gradient,
    GradientContainer & scratch_gradient,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  if (!_base_gradient_method || !_rhie_chow_user_object)
    mooseError("FVReconstructedPressureGradient '", name(), "' has not been set up.");

  const auto & rc = *_rhie_chow_user_object;
  const auto pressure_variable_number = rc.pressureVariableNumber();

  // This method replaces the pressure gradient used by Rhie-Chow, so every variable assigned to
  // this shared method must be that pressure variable.
  for (const auto variable_number : variable_numbers)
    if (variable_number != pressure_variable_number)
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' can only be used for the pressure variable registered on RhieChowMassFlux '",
                 _rhie_chow_user_object_name,
                 "'.");

  // During the first pressure-gradient update, Rhie-Chow has not yet reconstructed pressure
  // gradients from the face fluxes. Use the base method until those reconstructed gradients exist.
  if (!rc.hasReconstructedPressureGradient())
  {
    _base_gradient_method->computeGradient(
        system, output_gradient, scratch_gradient, variable_numbers);
    return;
  }

  // Start from the configured base gradient. It remains the fallback value and seeds the relaxed
  // feedback state the first time reconstructed gradients become available.
  _base_gradient_method->computeGradient(
      system, output_gradient, scratch_gradient, variable_numbers);

  const auto dim = system.feProblem().mesh().dimension();
  const Real alpha = rc.reconstructedPressureGradientFeedbackRelaxation();
  const auto & reconstructed_pressure_gradient = rc.reconstructedPressureGradientComponents();

  mooseAssert(reconstructed_pressure_gradient.size() == dim,
              "Reconstructed pressure-gradient container must match the mesh dimension.");

  bool reset_feedback = _reconstructed_gradient_feedback.size() != dim;
  if (!reset_feedback)
    for (const auto component : make_range(dim))
      if (_reconstructed_gradient_feedback[component]->size() != output_gradient[component]->size())
      {
        reset_feedback = true;
        break;
      }

  if (reset_feedback)
  {
    _reconstructed_gradient_feedback.clear();
    for (const auto component : make_range(dim))
      _reconstructed_gradient_feedback.push_back(output_gradient[component]->clone());
  }

  for (const auto component : make_range(dim))
  {
    mooseAssert(reconstructed_pressure_gradient[component]->size() ==
                    output_gradient[component]->size(),
                "Reconstructed and published pressure gradient vectors must have the same size.");

    // Relax the persistent feedback state toward the newly reconstructed value:
    // feedback = (1 - alpha) * feedback + alpha * reconstructed_gradient.
    auto & feedback = *_reconstructed_gradient_feedback[component];
    feedback.scale(1.0 - alpha);
    feedback.add(alpha, *reconstructed_pressure_gradient[component]);
    feedback.close();

    // Publish the feedback state. Unlike a fresh blend with the base gradient, this accumulates the
    // reconstructed pressure-gradient update over SIMPLE iterations.
    auto & gradient = *output_gradient[component];
    gradient = feedback;
  }
}
