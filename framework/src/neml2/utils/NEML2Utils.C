//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

std::string
stringify(MOOSEIOType type)
{
  switch (type)
  {
    case NEML2Utils::MOOSEIOType::TIME:
      return "TIME";
    case NEML2Utils::MOOSEIOType::SCALAR:
      return "SCALAR";
    case NEML2Utils::MOOSEIOType::FUNCTION:
      return "FUNCTION";
    case NEML2Utils::MOOSEIOType::VARIABLE:
      return "VARIABLE";
    case NEML2Utils::MOOSEIOType::MATERIAL:
      return "MATERIAL";
    default:
      mooseError("Unknown MOOSE IO type.");
  }
}

std::shared_ptr<neml2::Model>
getModel(neml2::Factory & factory, const std::string & name, neml2::Dtype dtype)
{
  const auto prev_dtype = neml2::get_default_dtype();
  neml2::set_default_dtype(dtype);
  auto model = factory.get_model(name);
  model->to(dtype);
  neml2::set_default_dtype(prev_dtype);
  return model;
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
