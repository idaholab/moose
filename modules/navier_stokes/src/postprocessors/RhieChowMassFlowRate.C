//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowMassFlowRate.h"

#include "MathFVUtils.h"

registerMooseObject("NavierStokesApp", RhieChowMassFlowRate);

InputParameters
RhieChowMassFlowRate::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Integrates the mass flux stored by a linear segregated Rhie-Chow user object over a "
      "boundary. If an advected quantity is supplied, the integrated value is the Rhie-Chow mass "
      "flux weighted by that quantity.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object", "The Rhie-Chow user object providing the face mass flux.");
  params.addParam<MooseFunctorName>("advected_quantity",
                                    "Optional quantity to weight by the Rhie-Chow mass flux.");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

RhieChowMassFlowRate::RhieChowMassFlowRate(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _adv_quant(isParamValid("advected_quantity") ? &getFunctor<ADReal>("advected_quantity")
                                                 : nullptr),
    _advected_interp_method(Moose::FV::InterpMethod::Upwind)
{
  _qp_integration = false;

  if (_adv_quant)
  {
    checkFunctorSupportsSideIntegration<ADReal>("advected_quantity", _qp_integration);
    Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
  }
}

Real
RhieChowMassFlowRate::computeQpIntegral()
{
  mooseError("RhieChowMassFlowRate only supports finite volume FaceInfo integration.");
}

Real
RhieChowMassFlowRate::computeFaceInfoIntegral(const FaceInfo * fi)
{
  mooseAssert(fi, "We should have a FaceInfo.");
  const auto face_mass_flux = _mass_flux_provider.getMassFlux(*fi);

  if (!_adv_quant)
    return face_mass_flux;

  const auto state = determineState();
  const bool correct_skewness =
      _advected_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage;

  mooseAssert(_adv_quant->hasFaceSide(*fi, true) || _adv_quant->hasFaceSide(*fi, false),
              "Advected quantity should be defined on one side of the face!");

  const auto * elem = _adv_quant->hasFaceSide(*fi, true) ? fi->elemPtr() : fi->neighborPtr();

  const auto adv_quant_face = MetaPhysicL::raw_value(
      (*_adv_quant)(Moose::FaceArg({fi,
                                    Moose::FV::limiterType(_advected_interp_method),
                                    face_mass_flux > 0,
                                    correct_skewness,
                                    elem,
                                    nullptr}),
                    state));
  return face_mass_flux * adv_quant_face;
}
