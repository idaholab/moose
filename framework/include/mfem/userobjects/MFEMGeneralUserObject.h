#pragma once

#include "MFEMProblem.h"

// MOOSE includes
#include "GeneralUserObject.h"

/*
 * This class adds a getMFEMProblem method.
 */
class MFEMGeneralUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMGeneralUserObject(const InputParameters & parameters);

  /// Returns a reference to the MFEMProblem instance.
  MFEMProblem & getMFEMProblem() const { return _mfem_problem; }

  void execute() override {}

  void initialize() override {}

  void finalize() override {}

private:
  MFEMProblem & _mfem_problem;
};