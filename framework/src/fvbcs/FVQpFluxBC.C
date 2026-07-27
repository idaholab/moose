//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVQpFluxBC.h"
#include "MooseVariableFV.h"
#include "SystemBase.h"

InputParameters
FVQpFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  return params;
}

FVQpFluxBC::FVQpFluxBC(const InputParameters & params)
  : FVFluxBC(params), _u(_var.adSln()), _u_neighbor(_var.adSlnNeighbor())
{
}

const ADReal &
FVQpFluxBC::uOnUSub() const
{
  mooseAssert(_face_info, "The face info has not been set");
  const auto ft = _face_info->faceType(std::make_pair(_var.number(), _var.sys().number()));
  mooseAssert(
      ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::NEIGHBOR,
      "The variable " << _var.name()
                      << " should be defined on exactly one adjacent subdomain for FVFluxBC "
                      << this->name());
  mooseAssert(_qp == 0,
              "At the time of writing, we only support one quadrature point, which should "
              "correspond to the location of the cell centroid. If that changes, we should "
              "probably change the body of FVQpFluxBC::uOnUSub");

  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    return _u[_qp];
  else
    return _u_neighbor[_qp];
}

const ADReal &
FVQpFluxBC::uOnGhost() const
{
  mooseAssert(_face_info, "The face info has not been set");
  const auto ft = _face_info->faceType(std::make_pair(_var.number(), _var.sys().number()));
  mooseAssert(
      ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::NEIGHBOR,
      "The variable " << _var.name()
                      << " should be defined on exactly one adjacent subdomain for FVFluxBC "
                      << this->name());
  mooseAssert(_qp == 0,
              "At the time of writing, we only support one quadrature point, which should "
              "correspond to the location of the cell centroid. If that changes, we should "
              "probably change the body of FVQpFluxBC::uOnGhost");

  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    return _u_neighbor[_qp];
  else
    return _u[_qp];
}
