//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
    var->requireQpComputations();

    if (tag_name == Moose::OLD_SOLUTION_TAG)
      var->sys().needSolutionState(1);
    else if (tag_name == Moose::OLDER_SOLUTION_TAG)
      var->sys().needSolutionState(2);

    const auto tag = _c_fe_problem.getVectorTagID(tag_name);

    const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

    variable.init(*var, tag);
  }
  else
  {
    std::vector<Real> default_value = {_c_parameters.defaultCoupledValue(var_name, comp)};

    variable.init(default_value, CoupleableKey{});
  }

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
      var->requireQpComputations();

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
Coupleable::kokkosCoupledVectorTagVectorVariable(const std::string & var_name,
                                                 const std::string & tag_name,
                                                 unsigned int comp) const
{
  Moose::Kokkos::Variable variable;

  if (isCoupled(var_name))
  {
    auto var = const_cast<MooseVariableField<RealVectorValue> *>(
        getVarHelper<MooseVariableField<RealVectorValue>>(var_name, comp));
    var->requireQpComputations();

    if (tag_name == Moose::OLD_SOLUTION_TAG)
      var->sys().needSolutionState(1);
    else if (tag_name == Moose::OLDER_SOLUTION_TAG)
      var->sys().needSolutionState(2);

    const auto tag = _c_fe_problem.getVectorTagID(tag_name);

    const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

    variable.init(*var, tag);
  }
  else
  {
    Moose::Kokkos::Real3 default_value = (*getDefaultVectorValue(var_name))[0];

    variable.init({default_value}, CoupleableKey{});
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::kokkosCoupledVectorTagVectorVariables(const std::string & var_name,
                                                  const std::string & tag_name) const
{
  Moose::Kokkos::Variable variable;

  const auto components = coupledComponents(var_name);

  if (isCoupled(var_name))
  {
    std::vector<const MooseVariableFieldBase *> vars;

    for (unsigned int comp = 0; comp < components; ++comp)
    {
      auto var = const_cast<MooseVariableField<RealVectorValue> *>(
          getVarHelper<MooseVariableField<RealVectorValue>>(var_name, comp));
      var->requireQpComputations();

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
    Moose::Kokkos::Real3 default_value = (*getDefaultVectorValue(var_name))[0];

    variable.init({default_value}, CoupleableKey{});
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::kokkosZeroVariable() const
{
  Moose::Kokkos::Variable variable;

  variable.init(std::vector<Real>{0}, CoupleableKey{});

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
  if (!isCoupled(var_name))
  {
    Moose::Kokkos::Variable variable;
    variable.init(std::vector<Real>{0}, CoupleableKey{});

    return Moose::Kokkos::VariableGradient(variable);
  }

  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VariableGradient(variable);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledVectorTagGradientsByName(const std::string & var_name,
                                                  const std::string & tag_name) const
{
  if (!isCoupled(var_name))
  {
    Moose::Kokkos::Variable variable;
    variable.init(std::vector<Real>(coupledComponents(var_name), 0), CoupleableKey{});

    return Moose::Kokkos::VariableGradient(variable);
  }

  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::VariableGradient(variable);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagNodalValueByName(const std::string & var_name,
                                                   const std::string & tag_name,
                                                   unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  if (variable.coupled() && !variable.nodal())
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

  if (variable.coupled() && !variable.nodal())
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

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagValues(const std::string & var_name,
                                         const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagValuesByName(var_name, tag_name);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledVectorTagGradient(const std::string & var_name,
                                           const std::string & tag_param_name,
                                           unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagGradientByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::kokkosCoupledVectorTagGradients(const std::string & var_name,
                                            const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagGradientsByName(var_name, tag_name);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagNodalValue(const std::string & var_name,
                                             const std::string & tag_param_name,
                                             unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagNodalValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagNodalValues(const std::string & var_name,
                                              const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagNodalValuesByName(var_name, tag_name);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagDofValue(const std::string & var_name,
                                           const std::string & tag_param_name,
                                           unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagDofValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableValue
Coupleable::kokkosCoupledVectorTagDofValues(const std::string & var_name,
                                            const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagDofValuesByName(var_name, tag_name);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorValueByName(const std::string & var_name,
                                                    const std::string & tag_name,
                                                    unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VectorVariableValue(variable);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorValuesByName(const std::string & var_name,
                                                     const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, tag_name);

  return Moose::Kokkos::VectorVariableValue(variable);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorTagVectorGradientByName(const std::string & var_name,
                                                       const std::string & tag_name,
                                                       unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VectorVariableGradient(variable);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorTagVectorGradientsByName(const std::string & var_name,
                                                        const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, tag_name);

  return Moose::Kokkos::VectorVariableGradient(variable);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorNodalValueByName(const std::string & var_name,
                                                         const std::string & tag_name,
                                                         unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, tag_name, comp);

  if (!variable.nodal())
    mooseError("Cannot get nodal values from the coupled vector variable '",
               var_name,
               "', because the associated variable is not nodal.");

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorNodalValuesByName(const std::string & var_name,
                                                          const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, tag_name);

  if (!variable.nodal())
    mooseError("Cannot get nodal values from the coupled vector variable '",
               var_name,
               "', because the associated variable is not nodal.");

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorDofValueByName(const std::string & var_name,
                                                       const std::string & tag_name,
                                                       unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorDofValuesByName(const std::string & var_name,
                                                        const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, tag_name);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorValue(const std::string & var_name,
                                              const std::string & tag_param_name,
                                              unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagVectorValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorValues(const std::string & var_name,
                                               const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagVectorValuesByName(var_name, tag_name);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorTagVectorGradient(const std::string & var_name,
                                                 const std::string & tag_param_name,
                                                 unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagVectorGradientByName(var_name, tag_name, comp);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorTagVectorGradients(const std::string & var_name,
                                                  const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagVectorGradientsByName(var_name, tag_name);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorNodalValue(const std::string & var_name,
                                                   const std::string & tag_param_name,
                                                   unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagVectorNodalValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorNodalValues(const std::string & var_name,
                                                    const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagVectorNodalValuesByName(var_name, tag_name);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorDofValue(const std::string & var_name,
                                                 const std::string & tag_param_name,
                                                 unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagVectorDofValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorTagVectorDofValues(const std::string & var_name,
                                                  const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName & tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosCoupledVectorTagVectorDofValuesByName(var_name, tag_name);
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

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorValue(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, _c_nodal);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorGradient(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableGradient(variable);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorNodalValue(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorDofValue(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorValues(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, _c_nodal);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorGradients(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableGradient(variable);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorNodalValues(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorDofValues(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorValueOld(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::OLD_SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, _c_nodal);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorGradientOld(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::OLD_SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableGradient(variable);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorNodalValueOld(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::OLD_SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorDofValueOld(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::OLD_SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorValuesOld(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::OLD_SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, _c_nodal);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorGradientsOld(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::OLD_SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableGradient(variable);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorNodalValuesOld(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::OLD_SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorDofValuesOld(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::OLD_SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorValueOlder(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::OLDER_SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, _c_nodal);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorGradientOlder(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::OLDER_SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableGradient(variable);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorNodalValueOlder(const std::string & var_name,
                                               unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::OLDER_SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorDofValueOlder(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::OLDER_SOLUTION_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorValuesOlder(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::OLDER_SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, _c_nodal);
}

Moose::Kokkos::VectorVariableGradient
Coupleable::kokkosCoupledVectorGradientsOlder(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::OLDER_SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableGradient(variable);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorNodalValuesOlder(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::OLDER_SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorDofValuesOlder(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::OLDER_SOLUTION_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorDot(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::SOLUTION_DOT_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, _c_nodal);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorNodalDot(const std::string & var_name, unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVectorVariable(var_name, Moose::SOLUTION_DOT_TAG, comp);

  return Moose::Kokkos::VectorVariableValue(variable, true);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorDots(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::SOLUTION_DOT_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, _c_nodal);
}

Moose::Kokkos::VectorVariableValue
Coupleable::kokkosCoupledVectorNodalDots(const std::string & var_name) const
{
  auto variable = kokkosCoupledVectorTagVectorVariables(var_name, Moose::SOLUTION_DOT_TAG);

  return Moose::Kokkos::VectorVariableValue(variable, true);
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
