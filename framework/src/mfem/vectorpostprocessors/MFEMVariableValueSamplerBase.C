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
    _interp_vals(_var_names.size(), mfem::Vector(points.size()))
{
  _vars.reserve(_var_names.size());
  _declared_vals.reserve(_var_names.size());

  for (const auto & var_name : _var_names)
  {
    auto & var = *getMFEMProblem().getGridFunction(var_name);
    _vars.push_back(var);

    // declare value vectors for outputting
    std::vector<std::reference_wrapper<VectorPostprocessorValue>> declared_vals;
    const auto val_dim = var.VectorDim();
    for (const auto i : make_range(val_dim))
    {
      auto & declared = this->declareVector(var_name + "_" + std::to_string(i));
      declared.resize(points.size());
      declared_vals.push_back(declared);
    }
    _declared_vals.push_back(declared_vals);
  }
}

int
MFEMVariableValueSamplerBase::getFESpaceContinuityType(size_t i_var) const
{
  return _vars[i_var].get().FESpace()->FEColl()->GetContType();
}

void
MFEMVariableValueSamplerBase::execute()
{
  for (size_t i_var = 0; i_var < _vars.size(); i_var++)
    _finder.Interpolate(_vars[i_var], _interp_vals[i_var]);
}

void
MFEMVariableValueSamplerBase::finalize()
{
  for (size_t i_var = 0; i_var < _vars.size(); i_var++)
  {
    _interp_vals[i_var].HostReadWrite();

    const auto val_dims = _vars[i_var].get().VectorDim();
    const auto num_points = _declared_points[0].get().size();
    const auto val_fespace_ordering = _vars[i_var].get().FESpace()->GetOrdering();
    for (const auto i_dim : make_range(val_dims))
      for (const auto i_point : make_range(num_points))
      {
        const auto mfem_idx =
            Moose::MFEM::MFEMIndex(i_dim, i_point, val_dims, num_points, val_fespace_ordering);
        _declared_vals[i_var][i_dim].get()[i_point] = _interp_vals[i_var][mfem_idx];
      }
  }
}

#endif // MOOSE_MFEM_ENABLED
