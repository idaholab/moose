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

InputParameters
MFEMValueSamplerBase::validParams()
{
  return MFEMSamplerBase::validParams();
}

MFEMValueSamplerBase::MFEMValueSamplerBase(const InputParameters & parameters,
                                           const std::vector<Point> & points)
  : MFEMSamplerBase(parameters, points),
    _var(*getMFEMProblem().getGridFunction(_var_name)),
    _interp_vals(points.size())
{
  const auto val_dim = _var.VectorDim();
  for (int i = 0; i < val_dim; i++)
  {
    auto & declared = this->declareVector(_var_name + "_" + std::to_string(i));
    declared.resize(points.size());
    _declared_vals.push_back(declared);
  }
}

void
MFEMValueSamplerBase::execute()
{
  _finder.Interpolate(_var, _interp_vals);
}

void
MFEMValueSamplerBase::finalizeValues()
{
  _interp_vals.HostReadWrite();

  const auto val_dims = _var.VectorDim();
  const auto num_points = _declared_points[0].get().size();
  const auto val_fespace_ordering = _var.FESpace()->GetOrdering();
  for (int i_dim = 0; i_dim < val_dims; i_dim++)
    for (size_t i_point = 0; i_point < num_points; i_point++)
    {
      const auto idx = mfemIndex(i_dim, i_point, val_dims, num_points, val_fespace_ordering);
      _declared_vals[i_dim].get()[i_point] = _interp_vals[idx];
    }
}

#endif // MOOSE_MFEM_ENABLED
