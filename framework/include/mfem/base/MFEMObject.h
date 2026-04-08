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
  /**
   * Declare the common parameters required by MFEM MooseObject-backed classes.
   */
  static InputParameters validParams();

  /**
   * Construct an MFEM object backed directly by MooseObject.
   */
  MFEMObject(const InputParameters & parameters);

  /**
   * Return the owning MFEM problem.
   */
  MFEMProblem & getMFEMProblem() { return _mfem_problem; }
  /**
   * Return the owning MFEM problem.
   */
  const MFEMProblem & getMFEMProblem() const { return _mfem_problem; }

  /**
   * Retrieve a scalar MFEM coefficient by its declared name.
   */
  mfem::Coefficient & getScalarCoefficientByName(const MFEMScalarCoefficientName & name);
  /**
   * Retrieve a vector MFEM coefficient by its declared name.
   */
  mfem::VectorCoefficient & getVectorCoefficientByName(const MFEMVectorCoefficientName & name);
  /**
   * Retrieve a matrix MFEM coefficient by its declared name.
   */
  mfem::MatrixCoefficient & getMatrixCoefficientByName(const MFEMMatrixCoefficientName & name);
  /**
   * Retrieve a scalar MFEM coefficient using the value of an input parameter.
   */
  mfem::Coefficient & getScalarCoefficient(const std::string & name);
  /**
   * Retrieve a vector MFEM coefficient using the value of an input parameter.
   */
  mfem::VectorCoefficient & getVectorCoefficient(const std::string & name);
  /**
   * Retrieve a matrix MFEM coefficient using the value of an input parameter.
   */
  mfem::MatrixCoefficient & getMatrixCoefficient(const std::string & name);

private:
  /// Owning MFEM problem for this object.
  MFEMProblem & _mfem_problem;
};

#endif
