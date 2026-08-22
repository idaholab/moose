//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVariableValueSamplerBase.h"

#include "MFEMProblem.h"
#include "MFEMVectorUtils.h"
#include "MooseError.h"

#include "mfem/fem/fespace.hpp"

InputParameters
MFEMVariableValueSamplerBase::validParams()
{
  return MFEMVariableSamplerBase::validParams();
}

MFEMVariableValueSamplerBase::MFEMVariableValueSamplerBase(const InputParameters & parameters,
                                                           const std::vector<Point> & points)
  : MFEMVariableSamplerBase(parameters, points),
    _var(*getMFEMProblem().getGridFunction(_var_name)),
    _interp_vals(points.size())
{
  // declare value vectors for outputting
  const auto val_dim = _var.VectorDim();
  for (const auto i : make_range(val_dim))
  {
    auto & declared = this->declareVector(_var_name + "_" + std::to_string(i));
    declared.resize(points.size());
    _declared_vals.push_back(declared);
  }
}

int
MFEMVariableValueSamplerBase::getFESpaceContinuityType() const
{
  return _var.FESpace()->FEColl()->GetContType();
}

void
MFEMVariableValueSamplerBase::execute()
{
  _finder.Interpolate(_var, _interp_vals);
}

void
MFEMVariableValueSamplerBase::finalizeValues()
{
  _interp_vals.HostReadWrite();

  const auto val_dims = _var.VectorDim();
  const auto num_points = _declared_points[0].get().size();
  const auto val_fespace_ordering = _var.FESpace()->GetOrdering();
  for (const auto i_dim : make_range(val_dims))
    for (const auto i_point : make_range(num_points))
    {
      const auto mfem_idx =
          Moose::MFEM::MFEMIndex(i_dim, i_point, val_dims, num_points, val_fespace_ordering);
      _declared_vals[i_dim].get()[i_point] = _interp_vals[mfem_idx];
    }
}

#endif // MOOSE_MFEM_ENABLED
