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
FVReconstructedPressureGradient::setupReconstructedGradientSource(
    const std::vector<std::unique_ptr<NumericVector<Number>>> & relaxed_source) const
{
  if (!_relaxed_gradient_source)
  {
    _relaxed_gradient_source = &relaxed_source;
    return;
  }

  mooseAssert(_relaxed_gradient_source == &relaxed_source,
              "Reconstructed gradient source must be set at most once.");
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

  const auto * relaxed_source = _relaxed_gradient_source;
  const bool has_reconstructed = relaxed_source && !relaxed_source->empty();

  if (!has_reconstructed)
  {
    // Before a reconstructed gradient is available, use the configured base gradient.
    _base_gradient_method->computeGradient(system, gradient, variable_numbers);
    return;
  }

  mooseAssert(relaxed_source->size() == gradient.size(),
              "Relaxed gradient container must match the output gradient size.");

  for (const auto component : index_range(gradient))
  {
    mooseAssert((*relaxed_source)[component]->size() == gradient[component]->size(),
                "Relaxed gradient and output gradient components must have the same size.");

    *gradient[component] = *(*relaxed_source)[component];
  }
}
