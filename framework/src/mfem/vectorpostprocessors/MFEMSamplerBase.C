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

#include "mfem/fem/fespace.hpp"

namespace
{
mfem::Vector
pointsToMFEMVector(const std::vector<Point> & points,
                   const unsigned int num_dims,
                   const mfem::Ordering::Type ordering)
{
  const unsigned int num_points = points.size();
  mfem::Vector mfem_points(num_points * num_dims);
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
    for (unsigned int i_dim = 0; i_dim < num_dims; i_dim++)
    {
      const size_t idx = MFEMSamplerBase::mfemIndex(i_dim, i_point, num_dims, num_points, ordering);
      mfem_points(idx) = points[i_point](i_dim);
    }
  return mfem_points;
}
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
  return params;
}

MFEMSamplerBase::MFEMSamplerBase(const InputParameters & parameters,
                                 const std::vector<Point> & points)
  : MFEMVectorPostprocessor(parameters),
    _var_name(getParam<VariableName>("variable")),
    _mesh(const_cast<mfem::ParMesh &>(getMFEMProblem().getMFEMVariableMesh(_var_name))),
    _finder(this->comm().get()),
    _points_ordering(getParam<MooseEnum>("point_ordering") == "NODES" ? mfem::Ordering::byNODES
                                                                      : mfem::Ordering::byVDIM),
    _points(pointsToMFEMVector(points, _mesh.SpaceDimension(), _points_ordering))
{
  if (getMFEMProblem().mesh().shouldDisplace())
    mooseError("MFEMSamplerBase does not yet support problems with displacement.");

  _mesh.EnsureNodes();
  _finder.Setup(_mesh);
  _finder.FindPoints(_points, _points_ordering);

  mfem::Array<unsigned int> point_codes = _finder.GetCode();
  for (size_t i = 0; i < points.size(); i++)
    if (point_codes[i] > 1)
      mooseError("MFEMSamplerBase could not find point at ", points[i], ".");

  const auto mesh_dim = _mesh.SpaceDimension();
  for (int i = 0; i < mesh_dim; i++)
  {
    auto & declared = this->declareVector("x_" + std::to_string(i));
    declared.resize(points.size());
    _declared_points.push_back(declared);
  }
}

void
MFEMSamplerBase::finalize()
{
  _points.HostReadWrite();

  const auto mesh_dim = _mesh.SpaceDimension();
  const auto num_points = _declared_points[0].get().size();
  for (int i_dim = 0; i_dim < mesh_dim; i_dim++)
    for (size_t i_point = 0; i_point < num_points; i_point++)
      _declared_points[i_dim].get()[i_point] =
          _points(mfemIndex(i_dim, i_point, mesh_dim, num_points, _points_ordering));

  finalizeValues();
}

size_t
MFEMSamplerBase::mfemIndex(
    size_t i_dim, size_t i_point, size_t num_dims, size_t num_points, mfem::Ordering::Type ordering)
{
  if (ordering == mfem::Ordering::byNODES)
    return i_dim * num_points + i_point;
  else
    return i_point * num_dims + i_dim;
}

#endif // MOOSE_MFEM_ENABLED
