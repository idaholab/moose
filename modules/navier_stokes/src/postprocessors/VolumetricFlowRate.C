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
#include <math.h>

registerMooseObject("NavierStokesApp", VolumetricFlowRate);

InputParameters
VolumetricFlowRate::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the volumetric flow rate of an advected quantity through an external boundary.");
  params.addParam<bool>("fv", false, "Whether finite volume variables are used");
  params.addRequiredCoupledVar("vel_x", "The x-axis velocity");
  params.addCoupledVar("vel_y", 0, "The y-axis velocity");
  params.addCoupledVar("vel_z", 0, "The z-axis velocity");
  params.addCoupledVar(
      "advected_variable", 0, "The advected variable quantity of which to study the flow");
  params.addParam<MooseFunctorName>(
      "advected_mat_prop", 0, "The advected material property of which to study the flow");
  params.addParam<MooseFunctorName>("advected_quantity",
                                    "The quantity to advect. This is the canonical parameter to "
                                    "set the advected quantity when finite volume is being used.");
  MooseEnum advected_interp_method("average upwind", "upwind");
  params.addParam<MooseEnum>("advected_interp_method",
                             advected_interp_method,
                             "The interpolation to use for the advected quantity. Options are "
                             "'upwind' and 'average', with the default being 'upwind'.");
  MooseEnum velocity_interp_method("average rc", "rc");
  params.addParam<MooseEnum>(
      "velocity_interp_method",
      velocity_interp_method,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");
  params.addParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  return params;
}

VolumetricFlowRate::VolumetricFlowRate(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
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
    mooseError("VolumetricFlowRatePostprocessor should be provided either an advected variable "
               "or an advected material property");

  // Check that the user isn't trying to get face values for material properties
  if (parameters.isParamSetByUser("advected_interp_method") && _advected_mat_prop_supplied)
    mooseWarning("Advected quantity interpolation methods are currently unavailable for "
                 "advected material properties.");

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

  const auto & advected_interp_method = getParam<MooseEnum>("advected_interp_method");
  if (advected_interp_method == "average")
    _advected_interp_method = InterpMethod::Average;
  else if (advected_interp_method == "upwind")
    _advected_interp_method = InterpMethod::Upwind;
  else
    mooseError("Unrecognized advected quantity interpolation type ",
               static_cast<std::string>(advected_interp_method));

  const auto & velocity_interp_method = getParam<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method == "average")
    _velocity_interp_method = InterpMethod::Average;
  else if (velocity_interp_method == "rc")
    _velocity_interp_method = InterpMethod::RhieChow;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(velocity_interp_method));
}

Real
VolumetricFlowRate::computeQpIntegral()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_fv)
  {
    // We should be at the edge of the domain
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");

    // Get face value for velocity
    const auto vel =
        MetaPhysicL::raw_value(_rc_uo->getVelocity(_velocity_interp_method, *fi, _tid));

    const auto elem_face = Moose::FV::elemFromFace(*_rc_uo, *fi);
    const auto neighbor_face = Moose::FV::neighborFromFace(*_rc_uo, *fi);

    Real adv_quant_interface;
    Moose::FV::interpolate(_advected_interp_method,
                           adv_quant_interface,
                           MetaPhysicL::raw_value((*_adv_quant)(elem_face)),
                           MetaPhysicL::raw_value((*_adv_quant)(neighbor_face)),
                           vel,
                           *fi,
                           true);
    return fi->normal() * vel * adv_quant_interface;
  }
  else
#endif
  {
    if (_advected_variable_supplied)
      return _advected_variable[_qp] * RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) *
             _normals[_qp];
    else if (_advected_mat_prop_supplied)
      return MetaPhysicL::raw_value(
                 _advected_material_property(std::make_tuple(_current_elem, _qp, _qrule))) *
             RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
    else
      return RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
  }
}
