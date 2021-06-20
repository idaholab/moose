//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumAdvection.h"

#include "INSFVPressureVariable.h"
#include "INSFVVelocityVariable.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "FVDirichletBC.h"
#include "INSFVFlowBC.h"
#include "INSFVFullyDevelopedFlowBC.h"
#include "INSFVNoSlipWallBC.h"
#include "INSFVSlipWallBC.h"
#include "INSFVSymmetryBC.h"
#include "INSFVAttributes.h"
#include "MooseUtils.h"

#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/vector_value.h"

#include <algorithm>

registerMooseObject("NavierStokesApp", INSFVMomentumAdvection);

std::unordered_map<const MooseApp *,
                   std::vector<std::unordered_map<const Elem *, VectorValue<ADReal>>>>
    INSFVMomentumAdvection::_rc_a_coeffs;

InputParameters
INSFVMomentumAdvection::validParams()
{
  InputParameters params = FVMatAdvection::validParams();
  params += UserObjectInterface::validParams();

  params.addRequiredCoupledVar("pressure", "The pressure variable.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");

  MooseEnum velocity_interp_method("average rc", "rc");

  params.addParam<MooseEnum>(
      "velocity_interp_method",
      velocity_interp_method,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");

  params.addRequiredParam<MaterialPropertyName>("mu", "The viscosity");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");

  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;

  params.addParam<UserObjectName>("velocity_interpolator", "Velocity interpolation user object");

  params.addClassDescription("Object for advecting momentum, e.g. rho*u");

  return params;
}

INSFVMomentumAdvection::INSFVMomentumAdvection(const InputParameters & params)
  : FVMatAdvection(params),
    UserObjectInterface(this),
    _p_var(dynamic_cast<const INSFVPressureVariable *>(getFieldVar("pressure", 0))),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _rho(getParam<Real>("rho")),
    _dim(_subproblem.mesh().dimension()),
    _velocity_interpolator(isParamValid("velocity_interpolator") ? &getUserObject<NSFVRhieChowInterpolator>("velocity_interpolator") : nullptr)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_p_var)
    paramError("pressure", "the pressure must be a INSFVPressureVariable.");

  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !params.isParamValid("w"))
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  const auto & velocity_interp_method = params.get<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method == "average")
    _velocity_interp_method = Moose::FV::InterpMethod::Average;
  else if (velocity_interp_method == "rc")
    _velocity_interp_method = Moose::FV::InterpMethod::RhieChow;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(velocity_interp_method));

  if (_tid == 0)
  {
    auto & vec_of_coeffs_map = _rc_a_coeffs[&_app];
    vec_of_coeffs_map.resize(libMesh::n_threads());
  }

  if (getParam<bool>("force_boundary_execution"))
    paramError("force_boundary_execution",
               "Do not use the force_boundary_execution parameter to control execution of INSFV "
               "advection objects");

  if (!getParam<std::vector<BoundaryName>>("boundaries_to_force").empty())
    paramError("boundaries_to_force",
               "Do not use the boundaries_to_force parameter to control execution of INSFV "
               "advection objects");

  // Add necessary object dependencies when
  if (_velocity_interpolator)
    _velocity_interpolator->addDependencies(this);
}

bool
INSFVMomentumAdvection::skipForBoundary(const FaceInfo & fi) const
{
  if (!onBoundary(fi))
    return false;

  // If we have flux bcs then we do skip
  const auto & flux_pr = _var.getFluxBCs(fi);
  if (flux_pr.first)
    return true;

  // If we have a flow boundary without a replacement flux BC, then we must not skip. Mass and
  // momentum are transported via advection across boundaries
  for (const auto bc_id : fi.boundaryIDs())
    if (_flow_boundaries.find(bc_id) != _flow_boundaries.end())
      return false;

  // If not a flow boundary, then there should be no advection/flow in the normal direction, e.g. we
  // should not contribute any advective flux
  return true;
}

ADReal
INSFVMomentumAdvection::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal adv_quant_interface;

  // Interpolate velocity on the face
  if (_velocity_interpolator)
    _velocity_interpolator->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  else
    Moose::FV::interpolate(_velocity_interp_method,
                           v,
                           _vel_elem[_qp],
                           _vel_neighbor[_qp],
                           *_face_info,
                           true);

  // Interpolate the advected quantity on the face
  Moose::FV::interpolate(_advected_interp_method,
                         adv_quant_interface,
                         _adv_quant_elem[_qp],
                         _adv_quant_neighbor[_qp],
                         v,
                         *_face_info,
                         true);
  return _normal * v * adv_quant_interface;
}

void
INSFVMomentumAdvection::clearRCCoeffs()
{
  auto it = _rc_a_coeffs.find(&_app);
  mooseAssert(it != _rc_a_coeffs.end(),
              "No RC coeffs structure exists for the given MooseApp pointer");
  mooseAssert(_tid < it->second.size(),
              "The RC coeffs structure size "
                  << it->second.size() << " is greater than or equal to the provided thread ID "
                  << _tid);
  it->second[_tid].clear();
}
