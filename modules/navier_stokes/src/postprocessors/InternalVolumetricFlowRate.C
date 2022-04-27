//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalVolumetricFlowRate.h"
#include "MathFVUtils.h"
#include "INSFVRhieChowInterpolator.h"
#include "NSFVUtils.h"
#include <math.h>

registerMooseObject("NavierStokesApp", InternalVolumetricFlowRate);

InputParameters
InternalVolumetricFlowRate::validParams()
{
  InputParameters params = InterfaceIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the volumetric flow rate of an advected quantity through an internal boundary.");
  params.addParam<bool>("fv", false, "Whether finite volume variables are used");
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

InternalVolumetricFlowRate::InternalVolumetricFlowRate(const InputParameters & parameters)
  : InterfaceIntegralPostprocessor(parameters),
    _fv(getParam<bool>("fv")),
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
    mooseError(
        "InternalVolumetricFlowRatePostprocessor should be provided either an advected variable or "
        "an advected material property");

  if (_fv)
  {
    if (!_rc_uo)
      mooseError("We were instructed to use finite volume, but no Rhie-Chow user object is "
                 "supplied. Please make sure to set the 'rhie_chow_user_object' parameter");
    if (!_adv_quant)
      mooseError("We were instructed to use finite volume, but no 'advected_quantity' parameter is "
                 "supplied.");
  }

  using namespace Moose::FV;
  setInterpolationMethods(*this, _advected_interp_method, _velocity_interp_method);
}

Real
InternalVolumetricFlowRate::computeQpIntegral()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_fv)
  {
    // We should not be at a boundary
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");
    mooseAssert(
        !fi->isBoundary(),
        "Use VolumetricFlowRate instead of InternalVolumetricFlowRate for domain boundaries");
    mooseAssert(_current_elem == &fi->elem(), "this must be true if the assertion on fi is true");

    // Get face value for velocity
    const auto vel =
        MetaPhysicL::raw_value(_rc_uo->getVelocity(_velocity_interp_method, *fi, _tid));
    const auto adv_quant_face = MetaPhysicL::raw_value((*_adv_quant)(
        Moose::FV::makeFace(*fi,
                            Moose::FV::limiterType(_advected_interp_method),
                            MetaPhysicL::raw_value(vel) * fi->normal() > 0,
                            std::make_pair(fi->elemSubdomainID(), fi->neighborSubdomainID()))));

    return fi->normal() * vel * adv_quant_face;
  }
  else
#endif
  {
    if (parameters().isParamSetByUser("advected_variable"))
      return _advected_variable[_qp] * RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) *
             _normals[_qp];
    else if (parameters().isParamSetByUser("advected_mat_prop"))
      return MetaPhysicL::raw_value(_advected_material_property(
                 std::make_tuple(_current_elem, _current_side, _qp, _qrule))) *
             RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
    else
      return RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
  }
}
