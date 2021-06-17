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
  InputParameters params = INSFVSymmetryBC::validParams();
  params.addClassDescription(
      "Implements a free slip boundary condition using a penalty formulation.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", 0, "The velocity in the y direction.");
  params.addCoupledVar("w", 0, "The velocity in the z direction.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this BC applies to.");
  params.addRequiredParam<MaterialPropertyName>("mu", "The viscosity");
  return params;
}

INSFVSymmetryVelocityBC::INSFVSymmetryVelocityBC(const InputParameters & params)
  : INSFVSymmetryBC(params),
    _u_elem(adCoupledValue("u")),
    _v_elem(adCoupledValue("v")),
    _w_elem(adCoupledValue("w")),
    _u_neighbor(adCoupledNeighborValue("u")),
    _v_neighbor(adCoupledNeighborValue("v")),
    _w_neighbor(adCoupledNeighborValue("w")),
    _comp(getParam<MooseEnum>("momentum_component")),
    _mu_elem(getADMaterialProperty<Real>("mu")),
    _mu_neighbor(getNeighborADMaterialProperty<Real>("mu")),
    _dim(_subproblem.mesh().dimension())
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
INSFVSymmetryVelocityBC::computeQpResidual()
{
  const bool use_elem = _face_info->faceType(_var.name()) == FaceInfo::VarFaceNeighbors::ELEM;
  const Point & cell_centroid =
      use_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const auto & u_C = use_elem ? _u_elem : _u_neighbor;
  const auto & v_C = use_elem ? _v_elem : _v_neighbor;
  const auto & w_C = use_elem ? _w_elem : _w_neighbor;

  // FIXME: interpolate mu to the boundary, see #16809
  const auto & mu_b = use_elem ? _mu_elem : _mu_neighbor;

  const auto d_perpendicular = std::abs((_face_info->faceCentroid() - cell_centroid) * _normal);

  // See Moukalled 15.150. Recall that we multiply by the area in the base class, so S_b ->
  // _normal.norm() here

  ADReal v_dot_n = u_C[_qp] * _normal(0);
  if (_dim > 1)
    v_dot_n += v_C[_qp] * _normal(1);
  if (_dim > 2)
    v_dot_n += w_C[_qp] + _normal(2);

  return 2. * mu_b[_qp] * _normal.norm() / d_perpendicular * v_dot_n * _normal(_comp);
}
