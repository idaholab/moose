//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KSPRightDiagonalScaleTest.h"

#include "FEProblemBase.h"
#include "MooseUtils.h"
#include "NonlinearSystemBase.h"

#include "petscsnes.h"

#include <limits>

registerMooseObject("MooseTestApp", KSPRightDiagonalScaleTest);

InputParameters
KSPRightDiagonalScaleTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Exercises the NonlinearSystemBase KSP right diagonal scaling developer API.");
  auto & execute_on = params.set<ExecFlagEnum>("execute_on", true);
  execute_on.addAvailableFlags(EXEC_PRE_KERNELS);
  execute_on = {EXEC_PRE_KERNELS};
  MooseEnum test_type("normal unrequested nonpositive nonfinite nonowned conflicting", "normal");
  params.addParam<MooseEnum>(
      "test_type", test_type, "The right diagonal scaling behavior to test.");
  params.addParam<bool>(
      "write_on_rank_zero_only", false, "Whether only rank zero writes its owned scale entries.");
  return params;
}

KSPRightDiagonalScaleTest::KSPRightDiagonalScaleTest(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _nl(_fe_problem.getNonlinearSystemBase(_sys.number())),
    _test_type(getParam<MooseEnum>("test_type").getEnum<TestType>()),
    _write_on_rank_zero_only(getParam<bool>("write_on_rank_zero_only"))
{
  if (_test_type == TestType::UNREQUESTED)
    return;

  _nl.requestKSPRightDiagonalScale();
  _nl.requestKSPRightDiagonalScale();
  if (!_nl.hasKSPRightDiagonalScale())
    mooseError("Requesting a KSP right diagonal scale did not mark it as requested.");
}

Real
KSPRightDiagonalScaleTest::expectedScale(const dof_id_type dof) const
{
  return (dof + _t_step) % 2 ? 1.0 : 2.0;
}

void
KSPRightDiagonalScaleTest::execute()
{
  const auto & solution = *_nl.currentSolution();
  const auto first = solution.first_local_index();
  const auto last = solution.last_local_index();

  if (first == last)
    mooseError("KSPRightDiagonalScaleTest requires at least one local degree of freedom.");

  switch (_test_type)
  {
    case TestType::UNREQUESTED:
      _nl.setKSPRightDiagonalScale(first, 2.0);
      return;
    case TestType::NONPOSITIVE:
      _nl.setKSPRightDiagonalScale(first, 0.0);
      return;
    case TestType::NONFINITE:
      _nl.setKSPRightDiagonalScale(first, std::numeric_limits<Real>::quiet_NaN());
      return;
    case TestType::NONOWNED:
      _nl.setKSPRightDiagonalScale(last, 2.0);
      return;
    case TestType::CONFLICTING:
      _nl.setKSPRightDiagonalScale(first, 2.0);
      _nl.setKSPRightDiagonalScale(first, 3.0);
      return;
    case TestType::NORMAL:
      break;
  }

  if (!_write_on_rank_zero_only || processor_id() == 0)
  {
    auto repeated_dof = last;
    for (const auto dof : make_range(first, last))
    {
      const auto scale = expectedScale(dof);
      if (scale != 1.0)
      {
        _nl.setKSPRightDiagonalScale(dof, scale);
        if (repeated_dof == last)
          repeated_dof = dof;
      }
    }

    if (repeated_dof != last)
      _nl.setKSPRightDiagonalScale(repeated_dof, expectedScale(repeated_dof));
  }

  _nl.closeKSPRightDiagonalScale();
  _nl.closeKSPRightDiagonalScale();
  checkKSPScale();
}

void
KSPRightDiagonalScaleTest::checkKSPScale() const
{
  KSP ksp;
  LibmeshPetscCall(SNESGetKSP(_nl.getSNES(), &ksp));

  Vec scale;
  LibmeshPetscCall(KSPGetRightDiagonalScale(ksp, &scale));
  if (!scale)
    mooseError("PETSc KSP does not have the requested right diagonal scale.");

  PetscInt first;
  PetscInt last;
  LibmeshPetscCall(VecGetOwnershipRange(scale, &first, &last));

  const PetscScalar * values;
  LibmeshPetscCall(VecGetArrayRead(scale, &values));
  bool mismatch = false;
  dof_id_type mismatch_dof = 0;
  Real mismatch_value = 0.0;
  Real mismatch_expected = 0.0;
  for (const auto local_i : make_range(last - first))
  {
    const auto dof = static_cast<dof_id_type>(first + local_i);
    const Real expected =
        !_write_on_rank_zero_only || processor_id() == 0 ? expectedScale(dof) : 1.0;
    const Real value = PetscRealPart(values[local_i]);
    if (!MooseUtils::relativeFuzzyEqual(value, expected, 1e-12))
    {
      mismatch = true;
      mismatch_dof = dof;
      mismatch_value = value;
      mismatch_expected = expected;
      break;
    }
  }
  LibmeshPetscCall(VecRestoreArrayRead(scale, &values));

  if (mismatch)
    mooseError("KSP right diagonal scale entry for DOF ",
               mismatch_dof,
               " is ",
               mismatch_value,
               " but should be ",
               mismatch_expected,
               ".");
}
