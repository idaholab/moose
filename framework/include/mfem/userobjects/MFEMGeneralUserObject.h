#ifdef MFEM_ENABLED

#pragma once

// MOOSE includes
#include "GeneralUserObject.h"
#include "mfem.hpp"

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
