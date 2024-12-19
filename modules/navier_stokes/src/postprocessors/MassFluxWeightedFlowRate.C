//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxWeightedFlowRate.h"
#include "MathFVUtils.h"
#include "INSFVRhieChowInterpolator.h"
#include "NSFVUtils.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"

#include <math.h>

registerMooseObject("NavierStokesApp", MassFluxWeightedFlowRate);

InputParameters
MassFluxWeightedFlowRate::validParams()
{
  InputParameters params = VolumetricFlowRate::validParams();
  params.addRequiredParam<MooseFunctorName>("density",
                                            "Provide a functor that returns the density.");
  params.addClassDescription("Computes the mass flux weighted average of the quantity "
                             "provided by advected_quantity over a boundary.");
  return params;
}

MassFluxWeightedFlowRate::MassFluxWeightedFlowRate(const InputParameters & parameters)
  : VolumetricFlowRate(parameters), _density(getFunctor<ADReal>("density")), _mdot(0)
{
  if (_qp_integration)
    mooseError("This object only works only with finite volume.");

  checkFunctorSupportsSideIntegration<ADReal>("density", _qp_integration);
}

void
MassFluxWeightedFlowRate::initialize()
{
  VolumetricFlowRate::initialize();
  _mdot = 0;
}

Real
MassFluxWeightedFlowRate::computeFaceInfoIntegral([[maybe_unused]] const FaceInfo * fi)
{
  mooseAssert(fi, "We should have a face info in " + name());
  mooseAssert(_adv_quant, "We should have an advected quantity in " + name());
  const auto state = determineState();

  // Get face value for velocity
  const auto face_flux = MetaPhysicL::raw_value(_rc_uo->getVolumetricFaceFlux(
      _velocity_interp_method, *fi, state, _tid, /*subtract_mesh_velocity=*/true));
  const bool correct_skewness =
      _advected_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage;

  mooseAssert(_adv_quant->hasFaceSide(*fi, true) || _adv_quant->hasFaceSide(*fi, true),
              "Advected quantity must be defined on one of the sides of the face!");
  mooseAssert((_adv_quant->hasFaceSide(*fi, true) == _density.hasFaceSide(*fi, true)) ||
                  (_adv_quant->hasFaceSide(*fi, false) == _density.hasFaceSide(*fi, false)),
              "Density must be defined at least on one of the sides where the advected quantity is "
              "defined!");

  const auto face_arg =
      Moose::FaceArg({fi,
                      Moose::FV::limiterType(_advected_interp_method),
                      face_flux > 0,
                      correct_skewness,
                      _adv_quant->hasFaceSide(*fi, true) ? fi->elemPtr() : fi->neighborPtr(),
                      nullptr});
  auto dens = _density(face_arg, state);
  const auto adv_quant_face = MetaPhysicL::raw_value(dens * (*_adv_quant)(face_arg, state));
  _mdot += fi->faceArea() * fi->faceCoord() * MetaPhysicL::raw_value(dens) * face_flux;
  return face_flux * adv_quant_face;
}

void
MassFluxWeightedFlowRate::threadJoin(const UserObject & y)
{
  VolumetricFlowRate::threadJoin(y);
  const auto & pps = static_cast<const MassFluxWeightedFlowRate &>(y);
  _mdot += pps._mdot;
}

Real
MassFluxWeightedFlowRate::getValue() const
{
  return _integral_value / _mdot;
}

void
MassFluxWeightedFlowRate::finalize()
{
  VolumetricFlowRate::finalize();
  gatherSum(_mdot);
}
