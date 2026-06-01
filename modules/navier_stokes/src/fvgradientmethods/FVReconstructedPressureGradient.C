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
  for (const auto variable_number : variable_numbers)
    if (variable_number != pressure_variable_number)
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' can only be used for the pressure variable registered on RhieChowMassFlux '",
                 _rhie_chow_user_object_name,
                 "'.");

  if (!rc.hasReconstructedCellVelocity())
  {
    _base_gradient_method->computeGradient(
        system, output_gradient, scratch_gradient, variable_numbers);
    return;
  }

  _base_gradient_method->computeGradient(
      system, output_gradient, scratch_gradient, variable_numbers);

  const auto dim = system.feProblem().mesh().dimension();
  const Real alpha = rc.reconstructedPressureGradientRelaxation();
  const auto & reconstructed_cell_velocity = rc.reconstructedCellVelocityComponents();
  const auto & HbyA = rc.HbyAComponents();
  const auto & Ainv = rc.AinvComponents();

  mooseAssert(reconstructed_cell_velocity.size() == dim,
              "Reconstructed cell velocity container must match the mesh dimension.");
  mooseAssert(HbyA.size() == dim, "HbyA container must match the mesh dimension.");
  mooseAssert(Ainv.size() == dim, "Ainv container must match the mesh dimension.");

  for (const auto component : make_range(dim))
  {
    mooseAssert(
        reconstructed_cell_velocity[component]->size() == output_gradient[component]->size(),
        "Reconstructed cell velocity and pressure gradient vectors must have the same size.");
    mooseAssert(HbyA[component]->size() == output_gradient[component]->size(),
                "HbyA and pressure gradient vectors must have the same size.");
    mooseAssert(Ainv[component]->size() == output_gradient[component]->size(),
                "Ainv and pressure gradient vectors must have the same size.");

    auto & reconstructed_gradient = *scratch_gradient[component];
    reconstructed_gradient = *reconstructed_cell_velocity[component];
    reconstructed_gradient.scale(-1.0);
    reconstructed_gradient.add(-1.0, *HbyA[component]);
    reconstructed_gradient.pointwise_divide(reconstructed_gradient, *Ainv[component]);

    auto & gradient = *output_gradient[component];
    gradient.scale(1.0 - alpha);
    gradient.add(alpha, reconstructed_gradient);
  }
}
