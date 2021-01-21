//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumetricFlowRate.h"
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
  params.addParam<MaterialPropertyName>(
      "advected_mat_prop", 0, "The advected material property of which to study the flow");
  return params;
}

VolumetricFlowRate::VolumetricFlowRate(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _fv(getParam<bool>("fv")),
    _vel_x(coupledValue("vel_x")),
    _vel_y(coupledValue("vel_y")),
    _vel_z(coupledValue("vel_z")),
    _advected_variable(coupledValue("advected_variable")),
    _advected_material_property(getADMaterialProperty<Real>("advected_mat_prop"))
{
  /// Check that at most one advected quantity has been provided
  mooseAssert(!parameters.isParamSetByUser("advected_variable") ||
                  !parameters.isParamSetByUser("advected_mat_prop"),
              "VolumetricFlowRatePostprocessor should be provided either an advected variable or "
              "an advected material property");
}

Real
VolumetricFlowRate::computeQpIntegral()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_fv)
  {
    /// We should be at the edge of the domain
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");
    if (!fi->isBoundary())
      mooseError("VolumetricFlowRate should only be used at boundaries");

    /// Get face value for velocity
    const auto & vx_face = !getFieldVar("vel_x", 0) ? _vel_x[_qp] :
        MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(
        getFieldVar("vel_x", 0))->getBoundaryFaceValue(*fi));

    const auto & vy_face = !getFieldVar("vel_y", 0) ? _vel_y[_qp] :
        MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("vel_y", 0))
                                                       ->getBoundaryFaceValue(*fi));

    const auto & vz_face = !getFieldVar("vel_z", 0) ? _vel_z[_qp] :
        MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("vel_z", 0))
                                                       ->getBoundaryFaceValue(*fi));

    /// Compute the advected quantity on the face
    Real advected_quantity;
    if (parameters().isParamSetByUser("advected_variable"))
    {
      advected_quantity = !getFieldVar("advected_variable", 0) ? _advected_variable[_qp] :
          MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(
          getFieldVar("advected_variable", 0))->getBoundaryFaceValue(*fi));
    }
    else if (parameters().isParamSetByUser("advected_mat_prop"))
    {
      /// The material property would not need to be interpolated since we are at
      /// an external boundary
      advected_quantity = MetaPhysicL::raw_value(_advected_material_property[_qp]);
    }
    else
      advected_quantity = 1;

    return advected_quantity * RealVectorValue(vx_face, vy_face, vz_face) * _normals[_qp];
  }
  else
#endif
    if (parameters().isParamSetByUser("advected_variable"))
      return _advected_variable[_qp] * RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) *
             _normals[_qp];
    else if (parameters().isParamSetByUser("advected_mat_prop"))
      return MetaPhysicL::raw_value(_advected_material_property[_qp]) *
             RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
    else
      return RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
}
