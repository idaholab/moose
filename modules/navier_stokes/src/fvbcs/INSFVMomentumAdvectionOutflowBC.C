//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumAdvectionOutflowBC.h"
#include "INSFVVelocityVariable.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NS.h"
#include "SystemBase.h"

registerMooseObject("NavierStokesApp", INSFVMomentumAdvectionOutflowBC);

InputParameters
INSFVMomentumAdvectionOutflowBC::validParams()
{
  InputParameters params = INSFVFluxBC::validParams();
  params += INSFVFullyDevelopedFlowBC::validParams();
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addClassDescription("Fully developed outflow boundary condition for advecting momentum. "
                             "This will impose a zero normal gradient on the boundary velocity.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density");
  return params;
}

INSFVMomentumAdvectionOutflowBC::INSFVMomentumAdvectionOutflowBC(const InputParameters & params)
  : INSFVFluxBC(params),
    INSFVFullyDevelopedFlowBC(params),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))),
    _w_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))),
    _dim(_subproblem.mesh().dimension()),
    _rho(getFunctor<ADReal>(NS::density))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");
}

void
INSFVMomentumAdvectionOutflowBC::gatherRCData(const FaceInfo & fi)
{
  using namespace Moose::FV;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());

  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  ADRealVectorValue v(_u_var->getBoundaryFaceValue(*_face_info));
  if (_v_var)
    v(1) = _v_var->getBoundaryFaceValue(*_face_info);
  if (_w_var)
    v(2) = _w_var->getBoundaryFaceValue(*_face_info);

  const auto & elem = (_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? _face_info->elem()
                                                                       : _face_info->neighbor();
  const auto boundary_face = singleSidedFaceArg();

  const auto rho_boundary = _rho(boundary_face);
  const auto eps_boundary = epsFunctor()(boundary_face);

  // This will tend to be an extrapolated boundary for the velocity in which case, when using two
  // term expansion, this boundary value will actually be a function of more than just the degree of
  // freedom at the cell centroid adjacent to the face, e.g. it can/will depend on surrounding cell
  // degrees of freedom as well
  auto var_boundary = _var(boundary_face);
  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  ADReal a = var_boundary.derivatives()[dof_number];
  a *= _normal * v * rho_boundary / eps_boundary;

  const auto strong_resid = _normal * v * rho_boundary / eps_boundary * var_boundary;

  _rc_uo.addToA((_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? &fi.elem() : fi.neighborPtr(),
                _index,
                a * (fi.faceArea() * fi.faceCoord()));

  processResidual(strong_resid * (fi.faceArea() * fi.faceCoord()));
}
