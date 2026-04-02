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
  params.addClassDescription("Base class for non-executed MFEM objects.");
  return params;
}

MFEMObject::MFEMObject(const InputParameters & parameters)
  : MooseObject(parameters),
    FunctionInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    ReporterInterface(this),
    _mfem_problem(
        static_cast<MFEMProblem &>(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem")))
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

#endif
