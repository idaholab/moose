//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexValueSamplerBase.h"
#include "MFEMProblem.h"
#include "MFEMVectorUtils.h"

InputParameters
MFEMComplexValueSamplerBase::validParams()
{
  return MFEMSamplerBase::validParams();
}

MFEMComplexValueSamplerBase::MFEMComplexValueSamplerBase(const InputParameters & parameters,
                                                         const std::vector<Point> & points)
  : MFEMSamplerBase(parameters, points),
    _var(*getMFEMProblem().getComplexGridFunction(_var_name)),
    _real_interp_vals(points.size()),
    _imag_interp_vals(points.size())
{
  const auto val_dim = _var.real().VectorDim();
  for (int i = 0; i < val_dim; i++)
  {
    auto & real_declared = this->declareVector(_var_name + "_real_" + std::to_string(i));
    real_declared.resize(points.size());
    _declared_real_vals.push_back(real_declared);

    auto & imag_declared = this->declareVector(_var_name + "_imag_" + std::to_string(i));
    imag_declared.resize(points.size());
    _declared_imag_vals.push_back(imag_declared);
  }
}

void
MFEMComplexValueSamplerBase::execute()
{
  _finder.Interpolate(_var.real(), _real_interp_vals);
  _finder.Interpolate(_var.imag(), _imag_interp_vals);
}

void
MFEMComplexValueSamplerBase::finalizeValues()
{
  _real_interp_vals.HostReadWrite();
  _imag_interp_vals.HostReadWrite();

  const auto val_dims = _var.real().VectorDim();
  const auto num_points = _declared_points[0].get().size();
  const auto val_fespace_ordering = _var.real().FESpace()->GetOrdering();
  for (int i_dim = 0; i_dim < val_dims; i_dim++)
    for (size_t i_point = 0; i_point < num_points; i_point++)
    {
      const auto idx =
          Moose::MFEM::MFEMIndex(i_dim, i_point, val_dims, num_points, val_fespace_ordering);
      _declared_real_vals[i_dim].get()[i_point] = _real_interp_vals[idx];
      _declared_imag_vals[i_dim].get()[i_point] = _imag_interp_vals[idx];
    }
}

#endif // MOOSE_MFEM_ENABLED
