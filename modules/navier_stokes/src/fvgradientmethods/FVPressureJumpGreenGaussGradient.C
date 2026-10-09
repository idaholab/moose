//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "FVPressureJumpGreenGaussGradient.h"

#include "LinearFVGradientReader.h"
#include "LinearSystem.h"
#include "MathFVUtils.h"
#include "PorousRhieChowMassFlux.h"
#include "PressureJumpInterface.h"

registerMooseObject("NavierStokesApp", FVPressureJumpGreenGaussGradient);

InputParameters
FVPressureJumpGreenGaussGradient::validParams()
{
  InputParameters params = FVGreenGaussGradient::validParams();
  params.suppressParameter<MooseEnum>("limiter");
  params.addClassDescription(
      "Computes a Green-Gauss pressure gradient without smearing prescribed baffle jumps.");
  return params;
}

FVPressureJumpGreenGaussGradient::FVPressureJumpGreenGaussGradient(const InputParameters & params)
  : FVGreenGaussGradient(params)
{
}

void
FVPressureJumpGreenGaussGradient::linkFlowSystem(PorousRhieChowMassFlux & rc,
                                                 const LinearFVGradientReader & pressure_gradient)
{
  if (&pressure_gradient.method() != this)
    mooseError("FVPressureJumpGreenGaussGradient '",
               name(),
               "' must be linked using a gradient field produced by that method.");

  if (pressure_gradient.system().number() != rc.pressureSystem().number() ||
      pressure_gradient.variableNumber() != rc.pressureVariableNumber())
    mooseError("FVPressureJumpGreenGaussGradient '",
               name(),
               "' must be linked to the pressure variable owned by PorousRhieChowMassFlux '",
               rc.name(),
               "'.");

  if (_rhie_chow && _rhie_chow != &rc)
    mooseError("FVPressureJumpGreenGaussGradient '",
               name(),
               "' is already linked to PorousRhieChowMassFlux '",
               _rhie_chow->name(),
               "'. Use a separate gradient method for each flow system.");

  _rhie_chow = &rc;
  _pressure_gradient = &pressure_gradient;
  _pressure_system = &pressure_gradient.system();
  _pressure_variable_number = pressure_gradient.variableNumber();
}

void
FVPressureJumpGreenGaussGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & gradient,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  mooseAssert(variable_numbers.size() == 1,
              "FVPressureJumpGreenGaussGradient must be registered for exactly one pressure "
              "variable.");

  if (_pressure_system)
  {
    mooseAssert(_pressure_system == &system,
                "FVPressureJumpGreenGaussGradient can only compute gradients for its linked "
                "pressure system.");
    mooseAssert(_pressure_variable_number == *variable_numbers.begin(),
                "FVPressureJumpGreenGaussGradient can only compute the linked pressure variable.");
  }

  computeGreenGaussGradient(system, gradient, variable_numbers, this);
}

FVTwoSidedFaceInterpolation::FaceValues
FVPressureJumpGreenGaussGradient::twoSidedInterpolate(const FaceInfo & fi,
                                                      const Real elem_value,
                                                      const Real neighbor_value) const
{
  if (!_rhie_chow || !_rhie_chow->faceIsBaffle(fi))
  {
    const Real face_value = Moose::FV::linearInterpolation(elem_value, neighbor_value, fi, true);
    return {face_value, face_value};
  }

  mooseAssert(_pressure_gradient, "A linked pressure gradient is required on pressure-jump faces.");

  const auto & elem_info = *fi.elemInfo();
  const auto & neighbor_info = *fi.neighborInfo();

  const Real jump = _rhie_chow->getSignedBaffleJump(fi, /*elem_side=*/true);
  if (!_rhie_chow->pressureDiffusionDataReady())
  {
    // Gradient history is initialized before the pressure-diffusion coefficients exist.
    // Preserve the jump during this startup pass using jump-adjusted interpolation.
    return {Moose::FV::linearInterpolation(elem_value, neighbor_value + jump, fi, true),
            Moose::FV::linearInterpolation(elem_value - jump, neighbor_value, fi, true)};
  }

  // The cell coefficients are the diagonal entries of the pressure-diffusion tensor.
  RealVectorValue elem_diffusion;
  RealVectorValue neighbor_diffusion;
  for (const auto component : make_range(_rhie_chow->dimension()))
  {
    elem_diffusion(component) = _rhie_chow->cellPressureDiffusionCoefficient(elem_info, component);
    neighbor_diffusion(component) =
        _rhie_chow->cellPressureDiffusionCoefficient(neighbor_info, component);
  }

  // Use the published geometric pressure gradient to match the deferred correction in the
  // pressure equation. The reconstructed coupling gradient must not enter this correction.
  const auto interface_data =
      NS::FV::pressureJumpInterfaceData(fi.normal(),
                                        fi.faceCentroid() - elem_info.centroid(),
                                        neighbor_info.centroid() - fi.faceCentroid(),
                                        elem_diffusion,
                                        neighbor_diffusion,
                                        _pressure_gradient->gradient(elem_info),
                                        _pressure_gradient->gradient(neighbor_info),
                                        fi.faceArea() * fi.faceCoord(),
                                        _rhie_chow->pressureDiffusionUsesNonorthogonalCorrection());

  if (interface_data.valid)
  {
    // Eliminate the interface pressures to obtain one conservative flux, then recover the
    // one-sided face value used in each cell's Green-Gauss surface sum.
    const Real flux = NS::FV::pressureJumpFlux(interface_data, elem_value, neighbor_value, jump);
    return {
        NS::FV::pressureJumpOneSidedFaceValue(interface_data, elem_value, flux, /*elem_side=*/true),
        NS::FV::pressureJumpOneSidedFaceValue(
            interface_data, neighbor_value, flux, /*elem_side=*/false)};
  }

  // Degenerate half-cell geometry cannot define a conductance. The interpolation
  // fallback still preserves the prescribed pressure jump.
  return {Moose::FV::linearInterpolation(elem_value, neighbor_value + jump, fi, true),
          Moose::FV::linearInterpolation(elem_value - jump, neighbor_value, fi, true)};
}
