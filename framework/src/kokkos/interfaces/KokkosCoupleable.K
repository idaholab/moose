//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVariableValue.h"

#include "Coupleable.h"
#include "SystemBase.h"
#include "FEProblemBase.h"

using CoupleableKey = Moose::Kokkos::Variable::CoupleableKey;

Moose::Kokkos::Variable
Coupleable::kokkosCoupledVectorTagVariable(const std::string & var_name,
                                           const std::string & tag_name,
                                           unsigned int comp) const
{
  Moose::Kokkos::Variable variable;

  if (isCoupled(var_name))
  {
    auto var = const_cast<MooseVariableField<Real> *>(
        getVarHelper<MooseVariableField<Real>>(var_name, comp));

    if (tag_name == Moose::OLD_SOLUTION_TAG)
      var->sys().needSolutionState(1);
    else if (tag_name == Moose::OLDER_SOLUTION_TAG)
      var->sys().needSolutionState(2);

    const auto tag = _c_fe_problem.getVectorTagID(tag_name);

    const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

    variable.init(*var, tag);
  }
  else
    variable.init({_c_parameters.defaultCoupledValue(var_name, comp)}, CoupleableKey{});

  return variable;
}

Moose::Kokkos::Variable
Coupleable::kokkosCoupledVectorTagVariables(const std::string & var_name,
                                            const std::string & tag_name) const
{
  Moose::Kokkos::Variable variable;

  const auto components = coupledComponents(var_name);

  if (isCoupled(var_name))
  {
    std::vector<const MooseVariableFieldBase *> vars;

    for (unsigned int comp = 0; comp < components; ++comp)
    {
      auto var = const_cast<MooseVariableField<Real> *>(
          getVarHelper<MooseVariableField<Real>>(var_name, comp));

      if (tag_name == Moose::OLD_SOLUTION_TAG)
        var->sys().needSolutionState(1);
      else if (tag_name == Moose::OLDER_SOLUTION_TAG)
        var->sys().needSolutionState(2);

      vars.push_back(var);
    }

    const auto tag = _c_fe_problem.getVectorTagID(tag_name);

    const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

    variable.init(vars, tag);
  }
  else
  {
    std::vector<Real> default_values(components);

    for (unsigned int comp = 0; comp < components; ++comp)
      default_values[comp] = _c_parameters.defaultCoupledValue(var_name, comp);

    variable.init(default_values, CoupleableKey{});
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::kokkosZeroVariable() const
{
  Moose::Kokkos::Variable variable;

  variable.init({0}, CoupleableKey{});

  return variable;
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagValueByName(const std::string & var_name,
                                              const std::string & tag_name,
                                              unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VariableValue(variable);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagValuesByName(const std::string & var_name,
                                               const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::VariableValue(variable);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledVectorTagGradientByName(const std::string & var_name,
                                                 const std::string & tag_name,
                                                 unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VariableGradient(variable);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledVectorTagGradientsByName(const std::string & var_name,
                                                  const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::VariableGradient(variable);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagNodalValueByName(const std::string & var_name,
                                                   const std::string & tag_name,
                                                   unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  if (!variable.nodal())
    mooseError("Cannot get nodal values from the coupled variable '",
               var_name,
               "', because the associated variable is not nodal.");

  return Moose::Kokkos::VariableValue(variable, true);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagNodalValuesByName(const std::string & var_name,
                                                    const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  if (!variable.nodal())
    mooseError("Cannot get nodal values from the coupled variable '",
               var_name,
               "', because the associated variable is not nodal.");

  return Moose::Kokkos::VariableValue(variable, true);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagDofValueByName(const std::string & var_name,
                                                 const std::string & tag_name,
                                                 unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VariableValue(variable, true);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagDofValuesByName(const std::string & var_name,
                                                  const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::VariableValue(variable, true);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagValue(const std::string & var_name,
                                        const std::string & tag_param_name,
                                        unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagValues(const std::string & var_name,
                                         const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagValuesByName(var_name, tag_name);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledVectorTagGradient(const std::string & var_name,
                                           const std::string & tag_param_name,
                                           unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagGradientByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledVectorTagGradients(const std::string & var_name,
                                            const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagGradientsByName(var_name, tag_name);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagNodalValue(const std::string & var_name,
                                             const std::string & tag_param_name,
                                             unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagNodalValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagNodalValues(const std::string & var_name,
                                              const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagNodalValuesByName(var_name, tag_name);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagDofValue(const std::string & var_name,
                                           const std::string & tag_param_name,
                                           unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagDofValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagDofValues(const std::string & var_name,
                                            const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagDofValuesByName(var_name, tag_name);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledValue(const std::string & var_name, unsigned int comp) const
{
  return _c_nodal ? kokkosCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_TAG, comp)
                  : kokkosCoupledVectorTagValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledValues(const std::string & var_name) const
{
  return _c_nodal ? kokkosCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_TAG)
                  : kokkosCoupledVectorTagValuesByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledGradient(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagGradientByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledGradients(const std::string & var_name) const
{
  return kokkosCoupledVectorTagGradientsByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledNodalValue(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledNodalValues(const std::string & var_name) const
{
  return kokkosCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledDofValue(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagDofValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledDofValues(const std::string & var_name) const
{
  return kokkosCoupledVectorTagDofValuesByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledValueOld(const std::string & var_name, unsigned int comp) const
{
  return _c_nodal ? kokkosCoupledVectorTagNodalValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp)
                  : kokkosCoupledVectorTagValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledValuesOld(const std::string & var_name) const
{
  return _c_nodal ? kokkosCoupledVectorTagNodalValuesByName(var_name, Moose::OLD_SOLUTION_TAG)
                  : kokkosCoupledVectorTagValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledGradientOld(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagGradientByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledGradientsOld(const std::string & var_name) const
{
  return kokkosCoupledVectorTagGradientsByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledNodalValueOld(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagNodalValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledNodalValuesOld(const std::string & var_name) const
{
  return kokkosCoupledVectorTagNodalValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledDofValueOld(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagDofValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledDofValuesOld(const std::string & var_name) const
{
  return kokkosCoupledVectorTagDofValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledValueOlder(const std::string & var_name, unsigned int comp) const
{
  return _c_nodal
             ? kokkosCoupledVectorTagNodalValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp)
             : kokkosCoupledVectorTagValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledValuesOlder(const std::string & var_name) const
{
  return _c_nodal ? kokkosCoupledVectorTagNodalValuesByName(var_name, Moose::OLDER_SOLUTION_TAG)
                  : kokkosCoupledVectorTagValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledGradientOlder(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagGradientByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledGradientsOlder(const std::string & var_name) const
{
  return kokkosCoupledVectorTagGradientsByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledNodalValueOlder(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagNodalValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledNodalValuesOlder(const std::string & var_name) const
{
  return kokkosCoupledVectorTagNodalValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledDofValueOlder(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagDofValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledDofValuesOlder(const std::string & var_name) const
{
  return kokkosCoupledVectorTagDofValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledDot(const std::string & var_name, unsigned int comp) const
{
  return _c_nodal ? kokkosCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp)
                  : kokkosCoupledVectorTagValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledDots(const std::string & var_name) const
{
  return _c_nodal ? kokkosCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_DOT_TAG)
                  : kokkosCoupledVectorTagValuesByName(var_name, Moose::SOLUTION_DOT_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledNodalDot(const std::string & var_name, unsigned int comp) const
{
  return kokkosCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledNodalDots(const std::string & var_name) const
{
  return kokkosCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_DOT_TAG);
}

Moose::Kokkos::Scalar<const Real>
Coupleable::kokkosCoupledDotDu(const std::string & var_name, unsigned int comp) const
{
  const auto var = getVar(var_name, comp);

  if (!var)
    return Moose::Kokkos::Scalar<const Real>(_kokkos_default_value_zero);
  else
    return Moose::Kokkos::Scalar<const Real>(var->sys().duDotDu(var->number() + comp));
}

Moose::Kokkos::VariableValue
Coupleable::kokkosZeroValue() const
{
  auto variable = kokkosZeroVariable();

  return Moose::Kokkos::VariableValue(variable);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosZeroGradient() const
{
  auto variable = kokkosZeroVariable();

  return Moose::Kokkos::VariableGradient(variable);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosZeroNodalValue() const
{
  auto variable = kokkosZeroVariable();

  return Moose::Kokkos::VariableValue(variable, true);
}
