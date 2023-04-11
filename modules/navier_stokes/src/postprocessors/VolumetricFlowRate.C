//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumetricFlowRate.h"
#include "MathFVUtils.h"
#include "INSFVRhieChowInterpolator.h"
#include "NSFVUtils.h"

#include <cmath>

registerMooseObject("NavierStokesApp", VolumetricFlowRate);

InputParameters
VolumetricFlowRate::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the volumetric flow rate of an advected quantity through a sideset.");
  params.addRequiredCoupledVar("vel_x", "The x-axis velocity");
  params.addCoupledVar("vel_y", 0, "The y-axis velocity");
  params.addCoupledVar("vel_z", 0, "The z-axis velocity");
  params.addCoupledVar("advected_variable",
                       0,
                       "The advected variable quantity of which to study the flow; useful for "
                       "finite element simulations");
  params.addParam<MooseFunctorName>("advected_mat_prop",
                                    0,
                                    "The advected material property of which to study the flow; "
                                    "useful for finite element simulations");
  params.addParam<MooseFunctorName>("advected_quantity",
                                    "The quantity to advect. This is the canonical parameter to "
                                    "set the advected quantity when finite volume is being used.");
  params += Moose::FV::interpolationParameters();
  params.addParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  return params;
}

VolumetricFlowRate::VolumetricFlowRate(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _vel_x(coupledValue("vel_x")),
    _vel_y(coupledValue("vel_y")),
    _vel_z(coupledValue("vel_z")),
    _advected_variable_supplied(parameters.isParamSetByUser("advected_variable")),
    _advected_variable(coupledValue("advected_variable")),
    _advected_mat_prop_supplied(parameters.isParamSetByUser("advected_mat_prop")),
    _advected_material_property(getFunctor<ADReal>("advected_mat_prop")),
    _adv_quant(isParamValid("advected_quantity") ? &getFunctor<ADReal>("advected_quantity")
                                                 : nullptr),
    _rc_uo(isParamValid("rhie_chow_user_object")
               ? &getUserObject<INSFVRhieChowInterpolator>("rhie_chow_user_object")
               : nullptr)
{
  // Check that at most one advected quantity has been provided
  if (_advected_variable_supplied && _advected_mat_prop_supplied)
    mooseError("VolumetricFlowRatePostprocessor should be provided either an advected variable "
               "or an advected material property");

  // Check that the user isn't trying to get face values for material properties
  if (parameters.isParamSetByUser("advected_interp_method") && _advected_mat_prop_supplied)
    mooseWarning("Advected quantity interpolation methods are currently unavailable for "
                 "advected material properties.");

  _qp_integration = !getFieldVar("vel_x", 0)->isFV();
  if (!_qp_integration)
  {
    if (!_rc_uo)
      mooseError("We were instructed to use finite volume, but no Rhie-Chow user object is "
                 "supplied. Please make sure to set the 'rhie_chow_user_object' parameter");
    if (!_adv_quant)
      mooseError("We were instructed to use finite volume, but no 'advected_quantity' parameter is "
                 "supplied.");

    Moose::FV::setInterpolationMethods(*this, _advected_interp_method, _velocity_interp_method);
  }
}

void
VolumetricFlowRate::initialSetup()
{
  if (_rc_uo && _rc_uo->velocityInterpolationMethod() == Moose::FV::InterpMethod::RhieChow)
  {
    // We must make sure the A coefficients in the Rhie Chow interpolator are present on
    // both sides of the boundaries so that interpolation coefficients may be computed
    for (const auto bid : boundaryIDs())
      const_cast<INSFVRhieChowInterpolator *>(_rc_uo)->ghostADataOnBoundary(bid);

    // On INITIAL, we cannot compute Rhie Chow coefficients on internal surfaces because
    // - the time integrator is not ready to compute time derivatives
    // - the setup routine is called too early for porosity functions to be initialized
    // We must check that the boundaries requested are all external
    if (getExecuteOnEnum().contains(EXEC_INITIAL))
      for (const auto bid : boundaryIDs())
      {
        if (!_mesh.isBoundaryFullyExternalToSubdomains(bid, _rc_uo->blockIDs()))
          paramError(
              "execute_on",
              "Boundary '",
              _mesh.getBoundaryName(bid),
              "' (id=",
              bid,
              ") has been detected to be internal to the flow domain.\n"
              "Volumetric flow rates cannot be computed on internal flow boundaries on INITIAL");
      }
  }
}

void
VolumetricFlowRate::meshChanged()
{
  initialSetup();
}

Real
VolumetricFlowRate::computeFaceInfoIntegral(const FaceInfo * fi)
{
  mooseAssert(fi, "We should have a face info in " + name());
  mooseAssert(_adv_quant, "We should have an advected quantity in " + name());
  const auto state = determineState();

  // Get face value for velocity
  const auto vel =
      MetaPhysicL::raw_value(_rc_uo->getVelocity(_velocity_interp_method, *fi, state, _tid));
  const bool correct_skewness =
      _advected_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage;

  const auto adv_quant_face = MetaPhysicL::raw_value(
      (*_adv_quant)(Moose::FaceArg({fi,
                                    Moose::FV::limiterType(_advected_interp_method),
                                    MetaPhysicL::raw_value(vel) * fi->normal() > 0,
                                    correct_skewness,
                                    nullptr}),
                    state));
  return fi->normal() * adv_quant_face * vel;
}

Real
VolumetricFlowRate::computeQpIntegral()
{
  if (_advected_variable_supplied)
    return _advected_variable[_qp] * RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) *
           _normals[_qp];
  else if (_advected_mat_prop_supplied)
    return MetaPhysicL::raw_value(_advected_material_property(
               std::make_tuple(_current_elem, _qp, _qrule), determineState())) *
           RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
  else
    return RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
}
