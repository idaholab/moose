//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addClassDescription("Fully developed outflow boundary condition for advecting momentum. "
                             "This will impose a zero normal gradient on the boundary velocity.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density");
  return params;
}

INSFVMomentumAdvectionOutflowBC::INSFVMomentumAdvectionOutflowBC(const InputParameters & params)
  : INSFVFluxBC(params),
    INSFVFullyDevelopedFlowBC(params),
    _u(getFunctor<ADReal>("u")),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _dim(_subproblem.mesh().dimension()),
    _rho(getFunctor<ADReal>(NS::density))
{
  if (_dim >= 2 && !_v)
    mooseError(
        "In two or more dimensions, the v velocity must be supplied using the 'v' parameter");
  if (_dim >= 3 && !_w)
    mooseError("In three dimensions, the w velocity must be supplied using the 'w' parameter");
}

ADReal
INSFVMomentumAdvectionOutflowBC::computeAdvectedQuantity(const Moose::FaceArg & boundary_face,
                                                         const Moose::StateArg & state)
{
  const auto rho_boundary = _rho(boundary_face, state);
  const auto eps_boundary = epsFunctor()(boundary_face, state);

  // This will tend to be an extrapolated boundary for the velocity in which case, when using two
  // term expansion, this boundary value will actually be a function of more than just the degree of
  // freedom at the cell centroid adjacent to the face, e.g. it can/will depend on surrounding cell
  // degrees of freedom as well
  const auto var_boundary = _var(boundary_face, state);

  return rho_boundary / eps_boundary * var_boundary;
}

ADReal
INSFVMomentumAdvectionOutflowBC::computeSegregatedContribution()
{
  const auto boundary_face = singleSidedFaceArg();
  const auto state = determineState();
  return _normal *
         _rc_uo.getVelocity(Moose::FV::InterpMethod::RhieChow,
                            *_face_info,
                            determineState(),
                            _tid,
                            /*subtract_mesh_velocity=*/true) *
         computeAdvectedQuantity(boundary_face, state);
}

void
INSFVMomentumAdvectionOutflowBC::gatherRCData(const FaceInfo & fi)
{
  using namespace Moose::FV;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(std::make_pair(_var.number(), _var.sys().number()));

  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  const auto boundary_face = singleSidedFaceArg();
  const auto state = determineState();
  ADRealVectorValue v(_u(boundary_face, state));
  if (_v)
    v(1) = (*_v)(boundary_face, state);
  if (_w)
    v(2) = (*_w)(boundary_face, state);

  const auto & elem = (_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? _face_info->elem()
                                                                       : _face_info->neighbor();

  // This will tend to be an extrapolated boundary for the velocity in which case, when using two
  // term expansion, this boundary value will actually be a function of more than just the degree of
  // freedom at the cell centroid adjacent to the face, e.g. it can/will depend on surrounding cell
  // degrees of freedom as well
  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  const auto advected_quant = computeAdvectedQuantity(boundary_face, state);
  const auto a = advected_quant.derivatives()[dof_number] * _normal * v;

  const auto strong_resid = _normal * v * advected_quant;

  _rc_uo.addToA((_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? fi.elemPtr() : fi.neighborPtr(),
                _index,
                a * (fi.faceArea() * fi.faceCoord()));

  addResidualAndJacobian(strong_resid * (fi.faceArea() * fi.faceCoord()));
}
