//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSamplerBase.h"
#include "MFEMProblem.h"
#include "MFEMVectorUtils.h"
#include "MooseError.h"

InputParameters
MFEMSamplerBase::validParams()
{
  InputParameters params = MFEMVectorPostprocessor::validParams();
  params.addParam<double>("mesh_boundary_tolerance",
                          1e-8,
                          "Distance from point to mesh boundary below which the point is "
                          "considered to be on the boundary rather than outside the mesh.");
  return params;
}

MFEMSamplerBase::MFEMSamplerBase(const InputParameters & parameters,
                                 const std::vector<Point> & points,
                                 mfem::ParMesh & mesh)
  : MFEMVectorPostprocessor(parameters),
    _query_points(points),
    _mesh(mesh),
    _finder(this->comm().get()),
    _points(Moose::MFEM::libMeshPointsToMFEMVector(
        points, _mesh.SpaceDimension(), mfem::Ordering::byVDIM))
{
  if (getMFEMProblem().mesh().shouldDisplace())
    mooseError("MFEMSamplerBase does not yet support problems with displacement.");

  _finder.SetDistanceToleranceForPointsFoundOnBoundary(getParam<double>("mesh_boundary_tolerance"));

  _mesh.EnsureNodes();
  _finder.Setup(_mesh);
  _finder.FindPoints(_points, mfem::Ordering::byVDIM);

  for (const auto i : make_range(_mesh.SpaceDimension()))
  {
    auto & declared = this->declareVector("x_" + std::to_string(i));
    declared.resize(points.size());
    _declared_points.push_back(declared);
  }
}

void
MFEMSamplerBase::initialSetup()
{
  const auto & point_codes = _finder.GetCode();
  for (const auto i : index_range(_query_points))
    if (PointLocationCode(point_codes[i]) == PointLocationCode::NOT_FOUND)
      mooseError(typeAndName(), " could not find point at ", _query_points[i], ".");

  for (const auto i_dim : index_range(_declared_points))
    for (const auto i_point : make_range(_query_points.size()))
      _declared_points[i_dim].get()[i_point] = _query_points[i_point](i_dim);
}

void
MFEMSamplerBase::finalize()
{
  finalizeValues();
}

#endif // MOOSE_MFEM_ENABLED
