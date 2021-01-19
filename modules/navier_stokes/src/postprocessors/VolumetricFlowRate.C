//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumetricFlowRate.h"
#include "FVUtils.h"
#include <math.h>

registerMooseObject("NavierStokesApp", VolumetricFlowRate);

InputParameters
VolumetricFlowRate::validParams()
{
  InputParameters params = InterfaceIntegralPostprocessor::validParams();
  params.addClassDescription("Computes the volumetric flow rate of an advected quantity through an internal boundary.");
  params.addParam<bool>("fv", false, "Whether finite volume variables are used");
  params.addRequiredCoupledVar("vel_x", "The x-axis velocity");
  params.addCoupledVar("vel_y", 0, "The y-axis velocity");
  params.addCoupledVar("vel_z", 0, "The z-axis velocity");
  MooseEnum advected_interp_method("average upwind", "upwind");
  params.addParam<MooseEnum>("advected_interp_method",
                             advected_interp_method,
                             "The interpolation to use for the advected quantity. Options are "
                             "'upwind' and 'average', with the default being 'upwind'.");
  params.addCoupledVar("advected_variable", 0, "The advected variable quantity of which to study the flow");
  params.addParam<MaterialPropertyName>("advected_mat_prop", 0,
                                        "The advected material property of which to study the flow");
  return params;
}

VolumetricFlowRate::VolumetricFlowRate(const InputParameters & parameters)
  : InterfaceIntegralPostprocessor(parameters),
    _fv(getParam<bool>("fv")),
    _vel_x(coupledValue("vel_x")),
    _vel_y(coupledValue("vel_y")),
    _vel_z(coupledValue("vel_z")),
    _advected_variable(coupledValue("advected_variable")),
    _advected_material_property(getADMaterialProperty<Real>("advected_mat_prop")),
    _advected_material_property_neighbor(getNeighborADMaterialProperty<Real>("advected_mat_prop"))
{
  /// Check that at most one advected quantity has been provided
  mooseAssert(!parameters.isParamSetByUser("advected_variable") ||
              !parameters.isParamSetByUser("advected_mat_prop"),
      "VolumetricFlowRatePostprocessor should be provided either an advected variable or an advected material property");

  using namespace Moose::FV;

  const auto & advected_interp_method = getParam<MooseEnum>("advected_interp_method");
  if (advected_interp_method == "average")
    _advected_interp_method = InterpMethod::Average;
  else if (advected_interp_method == "upwind")
    _advected_interp_method = InterpMethod::Upwind;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(advected_interp_method));
}

Real
VolumetricFlowRate::computeQpIntegral()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_fv)
  {
    /// We should not be at a boundary
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");
    if (fi->isBoundary())
      mooseError("Use BoundaryVolumetricFlowRate instead of VolumetricFlowRate for domain boundaries");

    /// Obtain the variable names from the parameters
    const auto & velx_name = parameters().getParamHelper("vel_x", parameters(),
        static_cast<std::vector<VariableName, std::allocator<VariableName> > *>(0));
    const auto & vely_name = parameters().getParamHelper("vel_y", parameters(),
        static_cast<std::vector<VariableName, std::allocator<VariableName> > *>(0));
    const auto & velz_name = parameters().getParamHelper("vel_z", parameters(),
        static_cast<std::vector<VariableName, std::allocator<VariableName> > *>(0));

    const Elem * neighbor = _current_elem->neighbor_ptr(_current_side);

    /// Get face value for velocity, using the variable's interpolation method
    const auto & vx_face = velx_name.empty() ? _vel_x[_qp] :
        MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(
        &_subproblem.getVariable(_tid, velx_name[0]))->getInternalFaceValue(neighbor, *fi, _vel_x[_qp]));

    const auto & vy_face = vely_name.empty() ? _vel_y[_qp] :
        MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(
        &_subproblem.getVariable(_tid, vely_name[0]))->getInternalFaceValue(neighbor, *fi, _vel_y[_qp]));

    const auto & vz_face = velz_name.empty() ? _vel_z[_qp] :
        MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(
        &_subproblem.getVariable(_tid, velz_name[0]))->getInternalFaceValue(neighbor, *fi, _vel_z[_qp]));

    /// Compute the advected quantity on the face
    Real advected_quantity;
    if (parameters().isParamSetByUser("advected_variable"))
    {
      const auto & advected_name = parameters().getParamHelper("advected_variable", parameters(),
          static_cast<std::vector<VariableName, std::allocator<VariableName> > *>(0));

      // If user did not request an interpolation method, use what the kernels are most likely to use
      if (!parameters().isParamSetByUser("advected_interp_method"))
      {
        advected_quantity = advected_name.empty() ? _advected_variable[_qp] :
            MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(
            &_subproblem.getVariable(_tid, advected_name[0]))->getInternalFaceValue(neighbor, *fi, _advected_variable[_qp]));
      }
      else
      {
        /// Get neighbor value
        const auto & advected_variable_neighbor = advected_name.empty() ? _advected_variable[_qp] :
            MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(
            &_subproblem.getVariable(_tid, advected_name[0]))->getNeighborValue(neighbor, *fi, _advected_variable[_qp]));

        Moose::FV::interpolate(_advected_interp_method,
                               advected_quantity,
                               MetaPhysicL::raw_value(_advected_variable[_qp]),
                               MetaPhysicL::raw_value(advected_variable_neighbor),
                               RealVectorValue(vx_face, vy_face, vz_face),
                               *fi,
                               true);
      }
    }
    else if (parameters().isParamSetByUser("advected_mat_prop"))
    {
      /// The material property needs to be interpolated since we are on an internal face
      Moose::FV::interpolate(_advected_interp_method,
                             advected_quantity,
                             MetaPhysicL::raw_value(_advected_material_property[_qp]),
                             MetaPhysicL::raw_value(_advected_material_property_neighbor[_qp]),
                             RealVectorValue(vx_face, vy_face, vz_face),
                             *fi,
                             true);
    }
    else
      advected_quantity = 1;

    return advected_quantity * RealVectorValue(vx_face, vy_face, vz_face) * _normals[_qp];
  }
  else
#endif
    if (parameters().isParamSetByUser("advected_variable"))
      return _advected_variable[_qp] * RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
    else if (parameters().isParamSetByUser("advected_mat_prop"))
      return MetaPhysicL::raw_value(_advected_material_property[_qp]) * RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
    else
      return RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
}
