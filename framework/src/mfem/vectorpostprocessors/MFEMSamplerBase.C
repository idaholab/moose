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
#include "MooseEnum.h"
#include "MooseError.h"

#include "mfem/fem/fespace.hpp"

namespace
{
CreateMooseEnumClass(L2AverageType, NONE, ARITHMETIC, HARMONIC);

/** Enum for values returned by GSLIB for point location relative to the mesh. */
enum class GSLibLocationCode : unsigned int
{
  INTERNAL = 0,
  BORDER = 1,
  NOT_FOUND = 2,
};
} // namespace

InputParameters
MFEMSamplerBase::validParams()
{
  InputParameters params = MFEMVectorPostprocessor::validParams();
  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "The names of the variables that this VectorPostprocessor operates on");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>(
      "point_ordering", ordering, "Ordering style to use for point vector DoFs.");
  MooseEnum avg_type(getL2AverageTypeOptions(), "ARITHMETIC", false);
  params.addParam<MooseEnum>("average_type",
                             avg_type,
                             "Average type used when sampling L2 functions at element boundaries.");
  params.addParam<double>("mesh_boundary_tolerance",
                          1e-8,
                          "Distance from point to mesh boundary below which the point is "
                          "considered to be on the boundary rather than outside the mesh.");
  return params;
}

MFEMSamplerBase::MFEMSamplerBase(const InputParameters & parameters,
                                 const std::vector<Point> & points)
  : MFEMVectorPostprocessor(parameters),
    _var_name(getParam<VariableName>("variable")),
    _query_points(points),
    _mesh(const_cast<mfem::ParMesh &>(getMFEMProblem().getMFEMVariableMesh(_var_name))),
    _finder(this->comm().get()),
    _points_ordering(getParam<MooseEnum>("point_ordering") == "NODES" ? mfem::Ordering::byNODES
                                                                      : mfem::Ordering::byVDIM),
    _points(
        Moose::MFEM::libMeshPointsToMFEMVector(points, _mesh.SpaceDimension(), _points_ordering))
{
  if (getMFEMProblem().mesh().shouldDisplace())
    mooseError("MFEMSamplerBase does not yet support problems with displacement.");

  _finder.SetL2AvgType(static_cast<mfem::FindPointsGSLIB::AvgType>(
      getParam<MooseEnum>("average_type").getEnum<L2AverageType>()));
  _finder.SetDistanceToleranceForPointsFoundOnBoundary(getParam<double>("mesh_boundary_tolerance"));

  _mesh.EnsureNodes();
  _finder.Setup(_mesh);
  _finder.FindPoints(_points, _points_ordering);

  const auto mesh_dim = _mesh.SpaceDimension();
  for (const auto i : make_range(mesh_dim))
  {
    auto & declared = this->declareVector("x_" + std::to_string(i));
    declared.resize(points.size());
    _declared_points.push_back(declared);
  }
}

void
MFEMSamplerBase::initialSetup()
{
  const bool fe_boundary_discontinuous = isFESpaceDiscontinuous();
  mfem::Array<unsigned int> point_codes = _finder.GetCode();
  for (const auto i : index_range(_query_points))
  {
    switch (GSLibLocationCode(point_codes[i]))
    {
      case GSLibLocationCode::INTERNAL:
        break;
      case GSLibLocationCode::BORDER:
        if (fe_boundary_discontinuous)
          mooseWarning("MFEMSamplerBase found a point on an element boundary but "
                       "the FE space is discontinuous at boundaries: ",
                       _query_points[i],
                       ".");
        break;
      default:
        mooseError("MFEMSamplerBase could not find point at ", _query_points[i], ".");
        break;
    }
  }
}

void
MFEMSamplerBase::finalize()
{
  _points.HostReadWrite();

  const auto mesh_dim = _mesh.SpaceDimension();
  const auto num_points = _declared_points[0].get().size();
  for (const auto i_dim : index_range(_declared_points))
    for (const auto i_point : index_range(_declared_points[i_dim].get()))
      _declared_points[i_dim].get()[i_point] =
          _points(Moose::MFEM::MFEMIndex(i_dim, i_point, mesh_dim, num_points, _points_ordering));

  finalizeValues();
}

#endif // MOOSE_MFEM_ENABLED
