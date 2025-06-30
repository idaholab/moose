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

#include "GeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

// Forwards declaration.
class MFEMProblem;

/*
 * This class adds a getMFEMProblem method.
 */
class MFEMGeneralUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMGeneralUserObject(const InputParameters & parameters);

  /// Returns a reference to the MFEMProblem instance.
  MFEMProblem & getMFEMProblem() { return _mfem_problem; }
  const MFEMProblem & getMFEMProblem() const { return _mfem_problem; }

  /// Returns references to coefficients stored in the MFEMProblem PropertiesManager.
  mfem::Coefficient & getScalarCoefficient(const std::string & name);
  mfem::VectorCoefficient & getVectorCoefficient(const std::string & name);
  mfem::MatrixCoefficient & getMatrixCoefficient(const std::string & name);

  void execute() override {}

  void initialize() override {}

  void finalize() override {}

private:
  MFEMProblem & _mfem_problem;
};

#endif
