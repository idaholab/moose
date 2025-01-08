//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2Utils.h"
#include "SubProblem.h"

namespace NEML2Utils
{
#ifdef NEML2_ENABLED
void
assertVariable(const neml2::VariableName & v)
{
  if (v.empty())
    mooseError("Empty NEML2 variable");

  if (!v.is_force() && !v.is_state())
    mooseError("The NEML2 variable '", v, "' is on the wrong subaxis.");
}

void
assertOldVariable(const neml2::VariableName & v)
{
  if (v.empty())
    mooseError("Empty NEML2 variable");

  if (!v.is_old_force() && !v.is_old_state())
    mooseError("The NEML2 variable '", v, "' is on the wrong subaxis.");
}

neml2::VariableName
parseVariableName(const std::string & s)
{
  return neml2::utils::parse<neml2::VariableName>(s);
}
#endif // NEML2_ENABLED

static const std::string missing_neml2 = "The `NEML2` library is required but not enabled. Refer "
                                         "to the documentation for guidance on how to enable it.";

bool
shouldCompute(const SubProblem & problem)
{
  // NEML2 computes residual and Jacobian together at EXEC_LINEAR
  // There is no work to be done at EXEC_NONLINEAR **UNLESS** we are computing the Jacobian for
  // automatic scaling.
  if (problem.computingScalingJacobian())
    return true;

  if (problem.currentlyComputingResidualAndJacobian())
    return true;

  if (problem.currentlyComputingJacobian())
    return false;

  return true;
}

std::string
docstring(const std::string & desc)
{
#ifndef NEML2_ENABLED
  return missing_neml2 + " (Original description: " + desc + ")";
#else
  return desc;
#endif
}

void
assertNEML2Enabled()
{
#ifndef NEML2_ENABLED
  mooseError(missing_neml2);
#endif
}

} // namespace NEML2Utils
