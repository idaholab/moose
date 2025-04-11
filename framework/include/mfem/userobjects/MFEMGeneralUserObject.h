#pragma once

// MOOSE includes
#include "GeneralUserObject.h"

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
  // FIXME: Should this be marked as const if it is returning a non-const reference?
  MFEMProblem & getMFEMProblem() const { return _mfem_problem; }

  void execute() override {}

  void initialize() override {}

  void finalize() override {}

private:
  MFEMProblem & _mfem_problem;
};
