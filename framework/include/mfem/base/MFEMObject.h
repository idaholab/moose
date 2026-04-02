//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MooseObject.h"
#include "FunctionInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "ReporterInterface.h"

class MFEMProblem;

/**
 * Thin base for MFEM objects backed directly by MooseObject instead of UserObject.
 */
class MFEMObject : public MooseObject,
                   protected FunctionInterface,
                   protected PostprocessorInterface,
                   protected VectorPostprocessorInterface,
                   protected ReporterInterface
{
public:
  static InputParameters validParams();

  MFEMObject(const InputParameters & parameters);

  MFEMProblem & getMFEMProblem() { return _mfem_problem; }
  const MFEMProblem & getMFEMProblem() const { return _mfem_problem; }

  mfem::Coefficient & getScalarCoefficientByName(const MFEMScalarCoefficientName & name);
  mfem::VectorCoefficient & getVectorCoefficientByName(const MFEMVectorCoefficientName & name);
  mfem::MatrixCoefficient & getMatrixCoefficientByName(const MFEMMatrixCoefficientName & name);
  mfem::Coefficient & getScalarCoefficient(const std::string & name);
  mfem::VectorCoefficient & getVectorCoefficient(const std::string & name);
  mfem::MatrixCoefficient & getMatrixCoefficient(const std::string & name);

protected:
  usingFunctionInterfaceMembers;
  usingPostprocessorInterfaceMembers;
  using ReporterInterface::getReporterValue;
  using ReporterInterface::getReporterValueByName;
  using VectorPostprocessorInterface::getVectorPostprocessorValue;
  using VectorPostprocessorInterface::getVectorPostprocessorValueByName;

private:
  MFEMProblem & _mfem_problem;
};

#endif
