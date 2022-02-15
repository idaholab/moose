//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVSymmetryVelocityBC.h"

registerMooseObject("NavierStokesApp", INSFVSymmetryVelocityBC);

InputParameters
INSFVSymmetryVelocityBC::validParams()
{
  InputParameters params = INSFVFluxBC::validParams();
  params += INSFVSymmetryBC::validParams();
  params.addClassDescription("Implements a symmetry boundary condition for the velocity.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", 0, "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", 0, "The velocity in the z direction.");
  params.addRequiredParam<MaterialPropertyName>("mu", "The viscosity");
  return params;
}

INSFVSymmetryVelocityBC::INSFVSymmetryVelocityBC(const InputParameters & params)
  : INSFVFluxBC(params),
    INSFVSymmetryBC(params),
    _u_functor(getFunctor<ADReal>("u")),
    _v_functor(getFunctor<ADReal>("v")),
    _w_functor(getFunctor<ADReal>("w")),
    _mu(getFunctor<ADReal>("mu")),
    _dim(_subproblem.mesh().dimension())
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

void
INSFVSymmetryVelocityBC::gatherRCData(const FaceInfo & fi)
{
  _face_info = &fi;
  _face_type = fi.faceType(_var.name());

  const bool use_elem = _face_info->faceType(_var.name()) == FaceInfo::VarFaceNeighbors::ELEM;
  const auto elem_arg =
      use_elem ? makeElemArg(&_face_info->elem()) : makeElemArg(_face_info->neighborPtr());
  const auto normal = use_elem ? _face_info->normal() : Point(-_face_info->normal());
  const Point & cell_centroid =
      use_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const auto u_C = _u_functor(elem_arg);
  const auto v_C = _v_functor(elem_arg);
  const auto w_C = _w_functor(elem_arg);

  const auto d_perpendicular = std::abs((_face_info->faceCentroid() - cell_centroid) * normal);

  // See Moukalled 15.150. Recall that we multiply by the area in the base class, so S_b ->
  // normal.norm() -> 1 here.

  const auto face = singleSidedFaceArg();
  const auto mu_b = _mu(face);

  ADReal v_dot_n = u_C * normal(0);
  if (_dim > 1)
    v_dot_n += v_C * normal(1);
  if (_dim > 2)
    v_dot_n += w_C * normal(2);

  const auto strong_resid = mu_b / d_perpendicular * normal(_index) * v_dot_n;

  // The strong residual for this object is a superposition of all the velocity components, however,
  // for the on-diagonal 'a' coefficient, we only care about the coefficient multiplying the
  // velocity component corresponding to _index, hence v_dot_n -> normal(_index) when moving from
  // strong_resid -> a
  const auto a = mu_b / d_perpendicular * normal(_index) * normal(_index);

  _rc_uo.addToA((_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? &fi.elem() : fi.neighborPtr(),
                _index,
                a * (fi.faceArea() * fi.faceCoord()));

  processResidual(strong_resid * (fi.faceArea() * fi.faceCoord()));
}
