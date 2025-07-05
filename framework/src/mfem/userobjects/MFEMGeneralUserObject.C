//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMGeneralUserObject.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGeneralUserObject);

InputParameters
MFEMGeneralUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Base class for custom GeneralUserObjects to add to MFEM problems.");
  return params;
}

MFEMGeneralUserObject::MFEMGeneralUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _mfem_problem(static_cast<MFEMProblem &>(_fe_problem))
{
}

mfem::Coefficient &
MFEMGeneralUserObject::getScalarCoefficientByName(const MFEMScalarCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getScalarCoefficient(name);
}

mfem::VectorCoefficient &
MFEMGeneralUserObject::getVectorCoefficientByName(const MFEMVectorCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getVectorCoefficient(name);
}

mfem::MatrixCoefficient &
MFEMGeneralUserObject::getMatrixCoefficientByName(const MFEMMatrixCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getMatrixCoefficient(name);
}

mfem::Coefficient &
MFEMGeneralUserObject::getScalarCoefficient(const std::string & name)
{
  return getScalarCoefficientByName(getParam<MFEMScalarCoefficientName>(name));
}

mfem::VectorCoefficient &
MFEMGeneralUserObject::getVectorCoefficient(const std::string & name)
{
  return getVectorCoefficientByName(getParam<MFEMVectorCoefficientName>(name));
}

mfem::MatrixCoefficient &
MFEMGeneralUserObject::getMatrixCoefficient(const std::string & name)
{
  return getMatrixCoefficientByName(getParam<MFEMMatrixCoefficientName>(name));
}

#endif
