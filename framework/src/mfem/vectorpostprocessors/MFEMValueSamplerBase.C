//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMValueSamplerBase.h"

#include "MFEMProblem.h"
#include "MFEMVectorUtils.h"
#include "MooseError.h"

#include "mfem/fem/fespace.hpp"

namespace
{
void
MFEMVectorToPostprocessorPoints(
    const mfem::Vector & mfem_points,
    std::vector<std::reference_wrapper<VectorPostprocessorValue>> & points,
    const unsigned int num_dims,
    const mfem::Ordering::Type ordering)
{
  const unsigned int num_points = mfem_points.Size() / num_dims;
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
  {
    for (unsigned int i_dim = 0; i_dim < num_dims; i_dim++)
    {
      const size_t idx = Moose::MFEM::MFEMIndex(i_dim, i_point, num_dims, num_points, ordering);

      points[i_dim].get()[i_point] = mfem_points(idx);
    }
  }
}

mfem::FindPointsGSLIB::AvgType
get_avg_type(const MooseEnum & avg_type)
{
  if (avg_type == "NONE")
  {
    return mfem::FindPointsGSLIB::AvgType::NONE;
  }
  else if (avg_type == "ARITHMETIC")
  {
    return mfem::FindPointsGSLIB::AvgType::ARITHMETIC;
  }
  else if (avg_type == "HARMONIC")
  {
    return mfem::FindPointsGSLIB::AvgType::HARMONIC;
  }
  else
  {
    mooseError("Unknown average type: ", avg_type);
  }
}
}

InputParameters
MFEMValueSamplerBase::validParams()
{
  InputParameters params = MFEMVectorPostprocessor::validParams();

  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "The names of the variables that this VectorPostprocessor operates on");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>(
      "point_ordering", ordering, "Ordering style to use for point vector DoFs.");
  MooseEnum avg_type("NONE ARITHMETIC HARMONIC", "ARITHMETIC", false);
  params.addParam<MooseEnum>(
      "average_type", avg_type, "Average type used when sampling L2 functions at boundaries.");
  params.addParam<double>("mesh_boundary_tolerance",
                          1e-8,
                          "Distance from point to mesh boundary below which the point is "
                          "considered to be on the boundary rather than outside the mesh.");

  return params;
}

MFEMValueSamplerBase::MFEMValueSamplerBase(const InputParameters & parameters,
                                           const std::vector<Point> & points)
  : MFEMVectorPostprocessor(parameters),
    _var_name(getParam<VariableName>("variable")),
    _var(*getMFEMProblem().getGridFunction(_var_name)),
    _mesh(const_cast<mfem::ParMesh &>(getMFEMProblem().getMFEMVariableMesh(_var_name))),
    _finder(this->comm().get()),
    _points_ordering(getParam<MooseEnum>("point_ordering") == "NODES" ? mfem::Ordering::byNODES
                                                                      : mfem::Ordering::byVDIM),
    _points(
        Moose::MFEM::libMeshPointsToMFEMVector(points, _mesh.SpaceDimension(), _points_ordering)),
    _interp_vals(points.size())
{
  if (getMFEMProblem().mesh().shouldDisplace())
    mooseError("MFEMValueSamplerBase does not yet support problems with displacement.");

  const mfem::FindPointsGSLIB::AvgType avg_type = get_avg_type(getParam<MooseEnum>("average_type"));
  _finder.SetL2AvgType(avg_type);
  _finder.SetDistanceToleranceForPointsFoundOnBoundary(getParam<double>("mesh_boundary_tolerance"));

  // set up points vector
  _mesh.EnsureNodes();
  _finder.Setup(_mesh);
  _finder.FindPoints(_points, _points_ordering);

  bool fe_boundary_discontinuous =
      _var.FESpace()->FEColl()->GetContType() == mfem::FiniteElementCollection::DISCONTINUOUS;

  // check all points were found
  mfem::Array<unsigned int> point_codes = _finder.GetCode();
  for (size_t i = 0; i < points.size(); i++)
  {
    switch (point_codes[i])
    {
      case 0:
        break;
      case 1:
        if (fe_boundary_discontinuous)
        {
          mooseWarning("MFEMValueSamplerBase found a point on an element boundary but "
                       "the FE space is discontinuous at boundaries: ",
                       points[i],
                       ".");
        }
        break;
      default:
        mooseError("MFEMValueSamplerBase could not find point at ", points[i], ".");
        break;
    }
  }

  // declare points vectors for outputting
  const auto mesh_dim = _mesh.SpaceDimension();
  for (int i = 0; i < mesh_dim; i++)
  {
    std::reference_wrapper<VectorPostprocessorValue> declared_dim =
        this->declareVector("x_" + std::to_string(i));
    declared_dim.get().resize(points.size());
    _declared_points.push_back(declared_dim);
  }

  // declare value vectors for outputting
  const auto val_dim = _var.VectorDim();
  for (int i = 0; i < val_dim; i++)
  {
    std::reference_wrapper<VectorPostprocessorValue> declared_dim =
        this->declareVector(_var_name + "_" + std::to_string(i));
    declared_dim.get().resize(points.size());
    _declared_vals.push_back(declared_dim);
  }
}

void
MFEMValueSamplerBase::execute()
{
  _finder.Interpolate(_var, _interp_vals);
}

void
MFEMValueSamplerBase::finalize()
{
  _interp_vals.HostReadWrite();
  _points.HostReadWrite();

  const auto mesh_dim = _mesh.SpaceDimension();
  MFEMVectorToPostprocessorPoints(_points, _declared_points, mesh_dim, _points_ordering);
  const auto val_dims = _var.VectorDim();
  const auto num_points = _declared_points[0].get().size();
  const auto val_fespace_ordering = _var.FESpace()->GetOrdering();
  for (int i_dim = 0; i_dim < val_dims; i_dim++)
  {
    for (size_t i_point = 0; i_point < num_points; i_point++)
    {
      const auto mfem_idx =
          Moose::MFEM::MFEMIndex(i_dim, i_point, val_dims, num_points, val_fespace_ordering);
      _declared_vals[i_dim].get()[i_point] = _interp_vals[mfem_idx];
    }
  }
}

#endif // MOOSE_MFEM_ENABLED
