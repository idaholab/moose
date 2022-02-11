//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RZSymmetry.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "FEProblemBase.h"
#include "libmesh/point.h"

InputParameters
RZSymmetry::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<Point>("axis_point", "A point on the axis of RZ symmetry.");
  params.addRequiredParam<RealVectorValue>("axis_dir", "The direction of the axis of RZ symmetry.");
  params.addParam<Real>("offset", 0., "Radial offset of the axis of RZ symmetry.");
  return params;
}

RZSymmetry::RZSymmetry(const MooseObject * moose_object, const InputParameters & parameters)
  : _axis_point(0., 0., 0.),
    _axis_dir(parameters.get<RealVectorValue>("axis_dir")),
    _offset(parameters.get<Real>("offset"))
{
  const FEProblemBase * fe_problem =
      moose_object->isParamValid("_fe_problem_base")
          ? moose_object->getParam<FEProblemBase *>("_fe_problem_base")
          : nullptr;
  if (fe_problem != nullptr)
  {
    const MooseMesh & mesh = fe_problem->mesh();

    const BlockRestrictable * blk_restr = dynamic_cast<const BlockRestrictable *>(moose_object);
    const BoundaryRestrictable * bnd_restr =
        dynamic_cast<const BoundaryRestrictable *>(moose_object);

    if (blk_restr != nullptr)
    {
      const std::set<SubdomainID> & blks = blk_restr->meshBlockIDs();

      for (auto & b : blks)
      {
        if (fe_problem->getCoordSystem(b) != Moose::COORD_XYZ)
          mooseError(
              moose_object->name(),
              ": This is a THM-specific object can be applied only on subdomains with Cartesian "
              "coordinate system. If your domain has RZ cooridnate system associated with it, you "
              "need to use the object without the RZ suffix to obtain the desired result.");
      }
    }
    if (bnd_restr != nullptr)
    {
      std::set<SubdomainID> blks;
      const std::set<BoundaryID> & bnds = bnd_restr->boundaryIDs();
      for (auto & bid : bnds)
      {
        std::set<SubdomainID> conn_blks = mesh.getBoundaryConnectedBlocks(bid);
        for (auto & cb : conn_blks)
          blks.insert(cb);
      }

      for (auto & b : blks)
      {
        if (fe_problem->getCoordSystem(b) != Moose::COORD_XYZ)
          mooseError(
              moose_object->name(),
              ": This is a THM-specific object can be applied only on subdomains with Cartesian "
              "coordinate system. If your domain has RZ cooridnate system associated with it, you "
              "need to use the object without the RZ suffix to obtain the desired result.");
      }
    }
  }

  const Point pt = parameters.get<Point>("axis_point");
  _axis_point = Point(pt(0), pt(1), pt(2));
}

Real
RZSymmetry::computeCircumference(const RealVectorValue & pt)
{
  RealVectorValue v = (pt - _axis_point);
  const Real r = v.cross(_axis_dir).norm() / _axis_dir.norm();
  return 2 * libMesh::pi * (r + _offset);
}
