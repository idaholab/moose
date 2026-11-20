//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObject.h"
#include "MFEMProblem.h"
#include "SubProblem.h"

InputParameters
MFEMObject::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += FunctionInterface::validParams();
  params += PostprocessorInterface::validParams();
  params += VectorPostprocessorInterface::validParams();
  params += ReporterInterface::validParams();
  params.addClassDescription("Base class for MFEM objects backed directly by MooseObject.");
  return params;
}

MFEMObject::MFEMObject(const InputParameters & parameters)
  : MooseObject(parameters),
    FunctionInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    ReporterInterface(this),
    _mfem_problem(
        cast_ref<MFEMProblem &>(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem")))
{
}

mfem::Coefficient &
MFEMObject::getScalarCoefficientByName(const MFEMScalarCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getScalarCoefficient(name);
}

mfem::VectorCoefficient &
MFEMObject::getVectorCoefficientByName(const MFEMVectorCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getVectorCoefficient(name);
}

mfem::MatrixCoefficient &
MFEMObject::getMatrixCoefficientByName(const MFEMMatrixCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getMatrixCoefficient(name);
}

mfem::Coefficient &
MFEMObject::getScalarCoefficient(const std::string & name)
{
  return getScalarCoefficientByName(getParam<MFEMScalarCoefficientName>(name));
}

mfem::VectorCoefficient &
MFEMObject::getVectorCoefficient(const std::string & name)
{
  return getVectorCoefficientByName(getParam<MFEMVectorCoefficientName>(name));
}

mfem::MatrixCoefficient &
MFEMObject::getMatrixCoefficient(const std::string & name)
{
  return getMatrixCoefficientByName(getParam<MFEMMatrixCoefficientName>(name));
}

std::variant<std::reference_wrapper<mfem::Coefficient>,
             std::reference_wrapper<mfem::DiagonalMatrixCoefficient>,
             std::reference_wrapper<mfem::MatrixCoefficient>>
MFEMObject::getVariantCoefficient(const std::string & name_scalar,
                                  const std::string & name_diagonal,
                                  const std::string & name_matrix)
{
  const bool scalar_set = isParamSetByUser(name_scalar);
  const bool diagonal_set = isParamSetByUser(name_diagonal);
  const bool matrix_set = isParamSetByUser(name_matrix);
  if (scalar_set + diagonal_set + matrix_set > 1)
  {
    paramError(name_scalar,
               "Specify only one of ",
               std::quoted(name_scalar),
               ", ",
               std::quoted(name_diagonal),
               ", and ",
               std::quoted(name_matrix),
               ".");
  }

  if (matrix_set)
  {
    return getMatrixCoefficient(name_matrix);
  }

  if (diagonal_set)
  {
    return getVectorCoefficient(name_diagonal);
  }

  // Use the scalar parameter's default when no coefficient is specified.
  return getScalarCoefficient(name_scalar);
}

#endif
