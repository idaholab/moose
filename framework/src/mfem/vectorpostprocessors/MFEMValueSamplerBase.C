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
#include "MFEMVectorFromlibMeshPoint.h"

#include "mfem/fem/fespace.hpp"

namespace {
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
}

InputParameters
MFEMValueSamplerBase::validParams()
{
  InputParameters params = MFEMVectorPostprocessor::validParams();

  params.addRequiredParam<VariableName>(
      "variable", "The names of the variables that this VectorPostprocessor operates on");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>(
      "point_ordering", ordering, "Ordering style to use for point vector DoFs.");

  return params;
}

MFEMValueSamplerBase::MFEMValueSamplerBase(const InputParameters & parameters,
                                           const std::vector<Point> & points)
  : MFEMVectorPostprocessor(parameters),
    _finder(this->comm().get()),
    _points_ordering(getParam<MooseEnum>("point_ordering") == "NODES" ? mfem::Ordering::byNODES
                                                                      : mfem::Ordering::byVDIM),
    _points(Moose::MFEM::pointsToMFEMVector(
        points, this->getMFEMProblem().mesh().getMFEMParMesh().SpaceDimension(), _points_ordering)),
    _interp_vals(points.size()),
    _var_name(getParam<VariableName>("variable")),
    _var(getMFEMProblem().getProblemData().gridfunctions.GetRef(_var_name))
{
  if (this->getMFEMProblem().mesh().shouldDisplace())
  {
    mooseError("MFEMValueSamplerBase does not yet support problems with displacement.");
  }

  // set up points vector
  auto & mesh = this->getMFEMProblem().mesh().getMFEMParMesh();
  mesh.EnsureNodes();
  _finder.Setup(mesh);
  _finder.FindPoints(_points, _points_ordering);

  // check all points were found
  mfem::Array<unsigned int> point_codes = _finder.GetCode();
  for (size_t i = 0; i < points.size(); i++)
  {
    if (point_codes[i] > 1)
    {
      mooseError("MFEMValueSamplerBase could not find point at ", points[i], ".");
    }
  }

  // declare points vectors for outputting
  const auto mesh_dim = this->getMFEMProblem().mesh().getMFEMParMesh().SpaceDimension();
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

  const auto mesh_dim = this->getMFEMProblem().mesh().getMFEMParMesh().SpaceDimension();
  MFEMVectorToPostprocessorPoints(_points, _declared_points, mesh_dim, _points_ordering);
  const auto val_dims = _var.VectorDim();
  const auto num_points = _declared_points[0].get().size();
  const auto val_fespace_ordering = _var.FESpace()->GetOrdering();
  for (int i_dim = 0; i_dim < val_dims; i_dim++)
  {
    for (size_t i_point = 0; i_point < num_points; i_point++)
    {
      const auto mfem_idx = Moose::MFEM::MFEMIndex(i_dim, i_point, val_dims, num_points, val_fespace_ordering);
      _declared_vals[i_dim].get()[i_point] = _interp_vals[mfem_idx];
    }
  }
}

#endif // MOOSE_MFEM_ENABLED
