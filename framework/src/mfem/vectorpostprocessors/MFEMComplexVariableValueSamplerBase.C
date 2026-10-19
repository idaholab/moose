//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVariableValueSamplerBase.h"
#include "MFEMProblem.h"
#include "MFEMVectorUtils.h"

#include "mfem/fem/fespace.hpp"

InputParameters
MFEMComplexVariableValueSamplerBase::validParams()
{
  return MFEMVariableSamplerBase::validParams();
}

MFEMComplexVariableValueSamplerBase::MFEMComplexVariableValueSamplerBase(
    const InputParameters & parameters, const std::vector<Point> & points)
  : MFEMVariableSamplerBase(parameters, points),
    _real_interp_vals(_var_names.size(), mfem::Vector(points.size())),
    _imag_interp_vals(_var_names.size(), mfem::Vector(points.size()))
{
  _vars.reserve(_var_names.size());
  _declared_real_vals.reserve(_var_names.size());
  _declared_imag_vals.reserve(_var_names.size());

  for (const auto & var_name : _var_names)
  {
    auto & var = *getMFEMProblem().getComplexGridFunction(var_name);
    _vars.push_back(var);

    std::vector<std::reference_wrapper<VectorPostprocessorValue>> declared_real_vals;
    std::vector<std::reference_wrapper<VectorPostprocessorValue>> declared_imag_vals;
    const auto val_dim = var.VectorDim();
    for (const auto i : make_range(val_dim))
    {
      auto & real_declared = this->declareVector(var_name + "_real_" + std::to_string(i));
      real_declared.resize(points.size());
      declared_real_vals.push_back(real_declared);

      auto & imag_declared = this->declareVector(var_name + "_imag_" + std::to_string(i));
      imag_declared.resize(points.size());
      declared_imag_vals.push_back(imag_declared);
    }
    _declared_real_vals.push_back(declared_real_vals);
    _declared_imag_vals.push_back(declared_imag_vals);
  }
}

int
MFEMComplexVariableValueSamplerBase::getFESpaceContinuityType(size_t i_var) const
{
  return _vars[i_var].get().FESpace()->FEColl()->GetContType();
}

void
MFEMComplexVariableValueSamplerBase::execute()
{
  for (size_t i_var = 0; i_var < _vars.size(); i_var++)
  {
    _finder.Interpolate(_vars[i_var].get().real(), _real_interp_vals[i_var]);
    _finder.Interpolate(_vars[i_var].get().imag(), _imag_interp_vals[i_var]);
  }
}

void
MFEMComplexVariableValueSamplerBase::finalize()
{
  for (size_t i_var = 0; i_var < _vars.size(); i_var++)
  {
    _real_interp_vals[i_var].HostReadWrite();
    _imag_interp_vals[i_var].HostReadWrite();

    const auto val_dims = _vars[i_var].get().VectorDim();
    const auto num_points = _declared_points[0].get().size();
    const auto val_fespace_ordering = _vars[i_var].get().FESpace()->GetOrdering();
    for (const auto i_dim : make_range(val_dims))
      for (const auto i_point : make_range(num_points))
      {
        const auto idx =
            Moose::MFEM::MFEMIndex(i_dim, i_point, val_dims, num_points, val_fespace_ordering);
        _declared_real_vals[i_var][i_dim].get()[i_point] = _real_interp_vals[i_var][idx];
        _declared_imag_vals[i_var][i_dim].get()[i_point] = _imag_interp_vals[i_var][idx];
      }
  }
}

#endif // MOOSE_MFEM_ENABLED
