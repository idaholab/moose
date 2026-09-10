//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarCoupleable.h"
#include "SystemBase.h"
#include "FEProblemBase.h"

using ScalarCoupleableKey = Moose::Kokkos::Variable::ScalarCoupleableKey;

Moose::Kokkos::Variable
ScalarCoupleable::kokkosCoupledVectorTagScalarVariable(const std::string & var_name,
                                                       const std::string & tag_name,
                                                       unsigned int comp) const
{
  Moose::Kokkos::Variable variable;

  if (isCoupledScalar(var_name, comp))
  {
    const auto var = const_cast<MooseVariableScalar *>(getScalarVar(var_name, comp));

    if (tag_name == Moose::OLD_SOLUTION_TAG)
      var->sys().needSolutionState(1);
    else if (tag_name == Moose::OLDER_SOLUTION_TAG)
      var->sys().needSolutionState(2);

    const auto tag = _sc_fe_problem.getVectorTagID(tag_name);
    _sc_coupleable_vector_tags.insert(tag);
    variable.init(*var, tag);
  }
  else
  {
    std::vector<Real> default_value(_sc_fe_problem.getMaxScalarOrder(),
                                    _sc_parameters.defaultCoupledValue(var_name));
    variable.init(default_value, ScalarCoupleableKey{});
  }

  return variable;
}

Moose::Kokkos::VariableValue
ScalarCoupleable::kokkosCoupledVectorTagScalarValueByName(const std::string & var_name,
                                                          const std::string & tag_name,
                                                          unsigned int comp) const
{
  checkVar(var_name);

  auto variable = kokkosCoupledVectorTagScalarVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VariableValue(variable);
}

Moose::Kokkos::VariableValue
ScalarCoupleable::kokkosCoupledVectorTagScalarValue(const std::string & var_name,
                                                    const std::string & tag_param_name,
                                                    unsigned int comp) const
{
  if (!_sc_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _sc_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagScalarValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableValue
ScalarCoupleable::kokkosCoupledScalarValue(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagScalarValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
ScalarCoupleable::kokkosCoupledScalarValueOld(const std::string & var_name, unsigned int comp) const
{
  if (isCoupledScalar(var_name, comp))
    validateExecutionerType(var_name, "kokkosCoupledScalarValueOld");

  return kokkosCoupledVectorTagScalarValueByName(
      var_name, _sc_is_implicit ? Moose::OLD_SOLUTION_TAG : Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
ScalarCoupleable::kokkosCoupledScalarValueOlder(const std::string & var_name,
                                                unsigned int comp) const
{
  if (isCoupledScalar(var_name, comp))
  {
    validateExecutionerType(var_name, "kokkosCoupledScalarValueOlder");

    if (!_sc_is_implicit)
      mooseError("Older values not available for explicit schemes");
  }

  return kokkosCoupledVectorTagScalarValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
ScalarCoupleable::kokkosCoupledScalarDot(const std::string & var_name, unsigned int comp) const
{
  validateExecutionerType(var_name, "kokkosCoupledScalarDot");

  return kokkosCoupledVectorTagScalarValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

Moose::Kokkos::Scalar<const Real>
ScalarCoupleable::kokkosCoupledScalarDotDu(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  validateExecutionerType(var_name, "kokkosCoupledScalarDotDu");

  auto var = getScalarVar(var_name, comp);
  return Moose::Kokkos::Scalar<const Real>(var->sys().duDotDu(var->number()));
}
