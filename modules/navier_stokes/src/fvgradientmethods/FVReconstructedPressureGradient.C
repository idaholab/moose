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
FVReconstructedPressureGradient::baseGradientMethod(SystemBase & system) const
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
FVReconstructedPressureGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & output_gradient,
    GradientContainer & scratch_gradient,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  const auto & rc = system.feProblem().getUserObject<RhieChowMassFlux>(_rhie_chow_user_object_name);

  for (const auto variable_number : variable_numbers)
    if (variable_number != rc.pressureVariableNumber())
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' can only be used for the pressure variable registered on RhieChowMassFlux '",
                 _rhie_chow_user_object_name,
                 "'.");

  if (!rc.hasReconstructedCellVelocity())
  {
    baseGradientMethod(system).computeGradient(
        system, output_gradient, scratch_gradient, variable_numbers);
    return;
  }

  baseGradientMethod(system).computeGradient(
      system, output_gradient, scratch_gradient, variable_numbers);

  for (auto & component : output_gradient)
    component->close();

  auto & mesh = system.feProblem().mesh();
  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!rc.hasFlowBlock(elem_info->subdomain_id()))
      continue;

    for (const auto variable_number : variable_numbers)
    {
      const auto dof = elem_info->dofIndices()[system.number()][variable_number];
      for (const auto component : make_range(mesh.dimension()))
      {
        const Real base_gradient = (*output_gradient[component])(dof);
        const Real Ainv = rc.Ainv(*elem_info, component);
        const Real reconstructed_gradient =
            Ainv != 0.0 ? (-rc.reconstructedCellVelocity(*elem_info, component) -
                           rc.HbyA(*elem_info, component)) /
                              Ainv
                        : base_gradient;
        const Real alpha = rc.reconstructedPressureGradientRelaxation();
        output_gradient[component]->set(
            dof, (1.0 - alpha) * base_gradient + alpha * reconstructed_gradient);
      }
    }
  }
}
