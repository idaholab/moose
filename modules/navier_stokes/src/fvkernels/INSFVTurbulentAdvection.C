//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTurbulentAdvection.h"

registerMooseObject("NavierStokesApp", INSFVTurbulentAdvection);

InputParameters
INSFVTurbulentAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription(
      "Advects an arbitrary turbulent quantity, the associated nonlinear 'variable'.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addParam<std::vector<BoundaryName>>("walls", "Boundaries that correspond to solid walls.");
  return params;
}

INSFVTurbulentAdvection::INSFVTurbulentAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  for (const auto & elem : _fe_problem.mesh().getMesh().element_ptr_range())
  {
    auto wall_bounded = false;
    for (unsigned int i_side = 0; i_side < elem->n_sides(); ++i_side)
    {
      const std::vector<BoundaryID> side_bnds = _subproblem.mesh().getBoundaryIDs(elem, i_side);
      for (const BoundaryName & name : _wall_boundary_names)
      {
        BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
        for (BoundaryID side_id : side_bnds)
        {
          if (side_id == wall_id)
            wall_bounded = true;
        }
      }
    }
    _wall_bounded[elem] = wall_bounded;
  }
}

ADReal
INSFVTurbulentAdvection::computeQpResidual()
{
  const auto v = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid);
  const auto var_face = _var(makeFace(
      *_face_info, limiterType(_advected_interp_method), MetaPhysicL::raw_value(v) * _normal > 0));
  const auto rho_face = _rho(makeFace(
      *_face_info, limiterType(_advected_interp_method), MetaPhysicL::raw_value(v) * _normal > 0));
  return _normal * MetaPhysicL::raw_value(v) * rho_face * var_face;
}

void
INSFVTurbulentAdvection::computeResidual(const FaceInfo & fi)
{

  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());
  auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  const Elem * elem = fi.elemPtr();
  const Elem * neighbor = fi.neighborPtr();
  auto bounded_elem = _wall_bounded[elem];
  auto bounded_neigh = _wall_bounded[neighbor];

  // _console << "Var Number: " << _var.number() << std::endl;
  // _console << "Face: " << fi.faceCentroid() << std::endl;
  // _console << "Elem: " << elem->vertex_average() << std::endl;
  // if (neighbor)
  //   _console << "Neigh: " << neighbor->vertex_average() << std::endl;
  // else
  //   _console << "No neighbor" << std::endl;
  // _console << "Elem bounded: " << bounded_elem << std::endl;
  // _console << "Neigh bounded: " << bounded_neigh << std::endl;

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    if (!bounded_elem)
    {
      // residual contribution of this kernel to the elem element
      prepareVectorTag(_assembly, _var.number());
      _local_re(0) = r;
      accumulateTaggedLocalResidual();
      // _console << "Assigning to element." << std::endl;
    }
  }
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    if (!bounded_neigh)
    {
      // residual contribution of this kernel to the neighbor element
      prepareVectorTagNeighbor(_assembly, _var.number());
      _local_re(0) = -r;
      accumulateTaggedLocalResidual();
      // _console << "Assigning to neighbor." << std::endl;
    }
  }
  // _console << "***********************" << std::endl;
}

void
INSFVTurbulentAdvection::computeJacobian(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());
  const ADReal r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  const Elem * elem = fi.elemPtr();
  const Elem * neighbor = fi.neighborPtr();
  auto bounded_elem = _wall_bounded[elem];
  auto bounded_neigh = _wall_bounded[neighbor];

  if ((_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
       _face_type == FaceInfo::VarFaceNeighbors::BOTH) &&
      (!bounded_elem))
  {
    mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

#ifdef MOOSE_GLOBAL_AD_INDEXING
    _assembly.processResidualAndJacobian(r, _var.dofIndices()[0], _vector_tags, _matrix_tags);
#else
    auto element_functor = [&](const ADReal & residual, dof_id_type, const std::set<TagID> &)
    {
      // jacobian contribution of the residual for the elem element to the elem element's DOF:
      // d/d_elem (residual_elem)
      computeJacobianType(Moose::ElementElement, residual);

      mooseAssert(
          (_face_type == FaceInfo::VarFaceNeighbors::ELEM) ==
              (_var.dofIndicesNeighbor().size() == 0),
          "If the variable is only defined on the elem hand side of the face, then that "
          "means it should have no dof indices on the neighbor/neighbor element. Conversely if "
          "the variable is defined on both sides of the face, then it should have a non-zero "
          "number of degrees of freedom on the neighbor/neighbor element");

      // only add residual to neighbor if the variable is defined there.
      if (_face_type == FaceInfo::VarFaceNeighbors::BOTH)
        // jacobian contribution of the residual for the elem element to the neighbor element's DOF:
        // d/d_neighbor (residual_elem)
        computeJacobianType(Moose::ElementNeighbor, residual);
    };
    _assembly.processJacobian(r, _var.dofIndices()[0], _matrix_tags, element_functor);
#endif
  }

  if ((_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
       _face_type == FaceInfo::VarFaceNeighbors::BOTH) &&
      (!bounded_neigh))
  {
    mooseAssert((_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR) ==
                    (_var.dofIndices().size() == 0),
                "If the variable is only defined on the neighbor hand side of the face, then that "
                "means it should have no dof indices on the elem element. Conversely if "
                "the variable is defined on both sides of the face, then it should have a non-zero "
                "number of degrees of freedom on the elem element");

    // We switch the sign for the neighbor residual
    ADReal neighbor_r = -r;

    mooseAssert(_var.dofIndicesNeighbor().size() == 1,
                "We're currently built to use CONSTANT MONOMIALS");

#ifdef MOOSE_GLOBAL_AD_INDEXING
    _assembly.processResidualAndJacobian(
        neighbor_r, _var.dofIndicesNeighbor()[0], _vector_tags, _matrix_tags);
#else
    auto neighbor_functor = [&](const ADReal & residual, dof_id_type, const std::set<TagID> &)
    {
      // only add residual to elem if the variable is defined there.
      if (_face_type == FaceInfo::VarFaceNeighbors::BOTH)
        // jacobian contribution of the residual for the neighbor element to the elem element's DOF:
        // d/d_elem (residual_neighbor)
        computeJacobianType(Moose::NeighborElement, residual);

      // jacobian contribution of the residual for the neighbor element to the neighbor element's
      // DOF: d/d_neighbor (residual_neighbor)
      computeJacobianType(Moose::NeighborNeighbor, residual);
    };

    _assembly.processJacobian(
        neighbor_r, _var.dofIndicesNeighbor()[0], _matrix_tags, neighbor_functor);
#endif
  }
}
