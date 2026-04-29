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

namespace Moose::MFEM
{
InputParameters
Object::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += FunctionInterface::validParams();
  params += PostprocessorInterface::validParams();
  params += VectorPostprocessorInterface::validParams();
  params += ReporterInterface::validParams();
  params.addClassDescription("Base class for MFEM objects backed directly by MooseObject.");
  return params;
}

Object::Object(const InputParameters & parameters)
  : MooseObject(parameters),
    FunctionInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    ReporterInterface(this),
    _mfem_problem(
        static_cast<Problem &>(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem")))
{
}

mfem::Coefficient &
Object::getScalarCoefficientByName(const Moose::MFEM::ScalarCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getScalarCoefficient(name);
}

mfem::VectorCoefficient &
Object::getVectorCoefficientByName(const Moose::MFEM::VectorCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getVectorCoefficient(name);
}

mfem::MatrixCoefficient &
Object::getMatrixCoefficientByName(const Moose::MFEM::MatrixCoefficientName & name)
{
  return getMFEMProblem().getCoefficients().getMatrixCoefficient(name);
}

mfem::Coefficient &
Object::getScalarCoefficient(const std::string & name)
{
  return getScalarCoefficientByName(getParam<Moose::MFEM::ScalarCoefficientName>(name));
}

mfem::VectorCoefficient &
Object::getVectorCoefficient(const std::string & name)
{
  return getVectorCoefficientByName(getParam<Moose::MFEM::VectorCoefficientName>(name));
}

mfem::MatrixCoefficient &
Object::getMatrixCoefficient(const std::string & name)
{
  return getMatrixCoefficientByName(getParam<Moose::MFEM::MatrixCoefficientName>(name));
}

} // namespace Moose::MFEM
#endif
