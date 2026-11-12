//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMatrixCoefficientFunction.h"
#include "MFEMProblem.h"
#include "MooseError.h"
#include "MooseUtils.h"

#include <algorithm>
#include <string>
#include <vector>

registerMooseObject("MooseApp", MFEMMatrixCoefficientFunction);

InputParameters
MFEMMatrixCoefficientFunction::validParams()
{
  auto params = Function::validParams();
  params.addClassDescription(
      "Declares a named MFEM matrix coefficient assembled from scalar coefficient names and/or "
      "numeric literals, evaluated at position and time.");
  params.addRequiredParam<std::vector<std::vector<MFEMScalarCoefficientName>>>(
      "matrix",
      "Nonempty, square, matrix of scalar coefficient names or numeric literals. Semicolons "
      "separate rows and whitespace separates entries. Dimensions are inferred.");
  return params;
}

MFEMMatrixCoefficientFunction::MFEMMatrixCoefficientFunction(const InputParameters & parameters)
  : Function(parameters),
    _entries(getParam<std::vector<std::vector<MFEMScalarCoefficientName>>>("matrix")),
    _mfem_problem(
        cast_ref<MFEMProblem &>(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem")))
{
  raise_on_invalid_matrix(_entries);
  _mfem_problem.getCoefficients().declareMatrix<mfem::MatrixArrayCoefficient>(name(),
                                                                              _entries.size());
}

void
MFEMMatrixCoefficientFunction::initialSetup()
{
  auto & manager = _mfem_problem.getCoefficients();
  auto & matrix = cast_ref<mfem::MatrixArrayCoefficient &>(manager.getMatrixCoefficient(name()));
  for (const auto i : index_range(_entries))
  {
    for (const auto j : index_range(_entries[i]))
    { // The manager owns the entries, which may be shared within or across matrices.
      matrix.Set(i, j, &manager.getScalarCoefficient(_entries[i][j]), false);
    }
  }
}

void
MFEMMatrixCoefficientFunction::raise_on_invalid_matrix(
    const std::vector<std::vector<MFEMScalarCoefficientName>> & mat)
{
  if (mat.empty())
  {
    paramError("matrix", "Matrix must not be empty.");
  }

  for (const auto i : index_range(mat))
  {
    if (mat[i].empty())
    {
      paramError("matrix", "Row ", i + 1, " has 0 entries (row indices are one-based).");
    }
    if (mat[i].size() != mat.front().size())
    {
      std::vector<std::string> row_sizes;
      std::transform(mat.begin(),
                     mat.end(),
                     std::back_inserter(row_sizes),
                     [](const auto & x) { return std::to_string(x.size()); });
      paramError("matrix",
                 "Not all rows have equal size; found row sizes {",
                 MooseUtils::join(row_sizes.begin(), row_sizes.end(), ", "),
                 "}");
    }
  }
  if (mat.size() != mat.front().size())
    paramError("matrix",
               "Matrix must be square, got ",
               mat.size(),
               " rows and ",
               mat.front().size(),
               " columns.");
}

#endif
