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
  const auto vel =
      MetaPhysicL::raw_value(_rc_uo->getVelocity(_velocity_interp_method, *fi, state, _tid));
  const bool correct_skewness =
      _advected_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage;

  const auto face_arg = Moose::FaceArg({fi,
                                        Moose::FV::limiterType(_advected_interp_method),
                                        MetaPhysicL::raw_value(vel) * fi->normal() > 0,
                                        correct_skewness,
                                        nullptr});
  auto dens = _density(face_arg, state);
  const auto adv_quant_face = MetaPhysicL::raw_value(dens * (*_adv_quant)(face_arg, state));
  _mdot += fi->faceArea() * fi->faceCoord() * MetaPhysicL::raw_value(dens) * fi->normal() * vel;
  return fi->normal() * adv_quant_face * vel;
}

void
MassFluxWeightedFlowRate::threadJoin(const UserObject & y)
{
  VolumetricFlowRate::threadJoin(y);
  const MassFluxWeightedFlowRate & pps = static_cast<const MassFluxWeightedFlowRate &>(y);
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
