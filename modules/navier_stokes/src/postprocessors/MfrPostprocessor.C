//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MfrPostprocessor.h"
#include "MooseMesh.h"
#include "libmesh/elem.h"

registerMooseObject("NavierStokesApp", MfrPostprocessor);

InputParameters
MfrPostprocessor::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription("Object for outputting boundary mass fluxes in conjunction with "
                             "FVFluxBC derived objects that support it");
  return params;
}

MfrPostprocessor::MfrPostprocessor(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters)
{
}

Real
MfrPostprocessor::computeQpIntegral()
{
  mooseError("We should never call this");
}

void
MfrPostprocessor::setMfr(const FaceInfo * const fi, const Real mfr, const bool includes_area)
{
  _fi_to_mfr[fi] = mfr * (includes_area ? Real(1) : (fi->faceArea() * fi->faceCoord()));
}

Real
MfrPostprocessor::computeIntegral()
{
  const FaceInfo * fi = _mesh.faceInfo(_current_elem, _current_side);
  if (!fi)
  {
    const Elem * const neighbor = _current_elem->neighbor_ptr(_current_side);
    mooseAssert(neighbor,
                "If current elem doesn't own a FaceInfo then there must be a neighbor who does.");
    const auto neigh_side = neighbor->which_neighbor_am_i(_current_elem);
    fi = _mesh.faceInfo(neighbor, neigh_side);
    mooseAssert(fi, "We must have found a FaceInfo by now");
  }
  auto it = _fi_to_mfr.find(fi);
  mooseAssert(it != _fi_to_mfr.end(), "We should have found the FaceInfo");
  return it->second;
}
