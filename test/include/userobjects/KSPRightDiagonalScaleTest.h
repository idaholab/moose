//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "GeneralUserObject.h"

class NonlinearSystemBase;

/**
 * Exercises the NonlinearSystemBase KSP right diagonal scaling developer API.
 */
class KSPRightDiagonalScaleTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  KSPRightDiagonalScaleTest(const InputParameters & parameters);

  void initialize() override {}
  void execute() override;
  void finalize() override {}

private:
  enum class TestType
  {
    NORMAL,
    UNREQUESTED,
    NONPOSITIVE,
    NONFINITE,
    NONOWNED,
    CONFLICTING
  };

  /// Return the scale expected for a DOF during the current timestep.
  Real expectedScale(dof_id_type dof) const;

  /// Check that PETSc has the MOOSE scale with the expected local entries.
  void checkKSPScale() const;

  NonlinearSystemBase & _nl;
  const TestType _test_type;
  const bool _write_on_rank_zero_only;
};
