//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUVariableValue.h"

#include "Coupleable.h"
#include "SystemBase.h"
#include "FEProblemBase.h"

Moose::Kokkos::Variable
Coupleable::KokkosCoupledVectorTagVariable(const std::string & var_name,
                                           const std::string & tag_name,
                                           unsigned int comp)
{
  Moose::Kokkos::Variable variable;

  const auto coupled = isCoupled(var_name);

  variable._coupled = coupled;
  variable._components = 1;

  if (coupled)
  {
    variable._var.create(1);
    variable._sys.create(1);

    auto var = const_cast<MooseVariableField<Real> *>(
        getVarHelper<MooseVariableField<Real>>(var_name, comp));

    if (tag_name == Moose::OLD_SOLUTION_TAG)
      var->sys().needSolutionState(1);
    else if (tag_name == Moose::OLDER_SOLUTION_TAG)
      var->sys().needSolutionState(2);

    variable._var[0] = var->number();
    variable._sys[0] = var->sys().number();

    variable._var.copy();
    variable._sys.copy();

    variable._tag = _c_fe_problem.getVectorTagID(tag_name);

    addFEVariableCoupleableVectorTag(variable._tag);
  }
  else
  {
    variable._default_value.create(1);
    variable._default_value[0] = _c_parameters.defaultCoupledValue(var_name, comp);
    variable._default_value.copy();
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::KokkosCoupledVectorTagVariables(const std::string & var_name,
                                            const std::string & tag_name)
{
  Moose::Kokkos::Variable variable;

  const auto coupled = isCoupled(var_name);
  const auto components = coupledComponents(var_name);

  variable._coupled = coupled;
  variable._components = components;

  if (coupled)
  {
    variable._var.create(components);
    variable._sys.create(components);

    for (unsigned int comp = 0; comp < components; ++comp)
    {
      auto var = const_cast<MooseVariableField<Real> *>(
          getVarHelper<MooseVariableField<Real>>(var_name, comp));

      if (tag_name == Moose::OLD_SOLUTION_TAG)
        var->sys().needSolutionState(1);
      else if (tag_name == Moose::OLDER_SOLUTION_TAG)
        var->sys().needSolutionState(2);

      variable._var[comp] = var->number();
      variable._sys[comp] = var->sys().number();
    }

    variable._var.copy();
    variable._sys.copy();

    variable._tag = _c_fe_problem.getVectorTagID(tag_name);

    addFEVariableCoupleableVectorTag(variable._tag);
  }
  else
  {
    variable._default_value.create(components);

    for (unsigned int comp = 0; comp < components; ++comp)
      variable._default_value[comp] = _c_parameters.defaultCoupledValue(var_name, comp);

    variable._default_value.copy();
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::KokkosZeroVariable()
{
  Moose::Kokkos::Variable variable;

  variable._default_value.create(1);
  variable._default_value[0] = 0;
  variable._default_value.copy();

  return variable;
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledVectorTagValueByName(const std::string & var_name,
                                              const std::string & tag_name,
                                              unsigned int comp)
{
  auto variable = KokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VariableValue(variable);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledVectorTagValuesByName(const std::string & var_name,
                                               const std::string & tag_name)
{
  auto variable = KokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::VariableValue(variable);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledVectorTagGradientByName(const std::string & var_name,
                                                 const std::string & tag_name,
                                                 unsigned int comp)
{
  auto variable = KokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VariableGradient(variable);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledVectorTagGradientsByName(const std::string & var_name,
                                                  const std::string & tag_name)
{
  auto variable = KokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::VariableGradient(variable);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledVectorTagNodalValueByName(const std::string & var_name,
                                                   const std::string & tag_name,
                                                   unsigned int comp)
{
  auto variable = KokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::VariableNodalValue(_c_fe_problem.getKokkosSystems(), variable);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledVectorTagNodalValuesByName(const std::string & var_name,
                                                    const std::string & tag_name)
{
  auto variable = KokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::VariableNodalValue(_c_fe_problem.getKokkosSystems(), variable);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledVectorTagValue(const std::string & var_name,
                                        const std::string & tag_param_name,
                                        unsigned int comp)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return KokkosCoupledVectorTagValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledVectorTagValues(const std::string & var_name,
                                         const std::string & tag_param_name)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return KokkosCoupledVectorTagValuesByName(var_name, tag_name);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledVectorTagGradient(const std::string & var_name,
                                           const std::string & tag_param_name,
                                           unsigned int comp)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return KokkosCoupledVectorTagGradientByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledVectorTagGradients(const std::string & var_name,
                                            const std::string & tag_param_name)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return KokkosCoupledVectorTagGradientsByName(var_name, tag_name);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledVectorTagNodalValue(const std::string & var_name,
                                             const std::string & tag_param_name,
                                             unsigned int comp)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return KokkosCoupledVectorTagNodalValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledVectorTagNodalValues(const std::string & var_name,
                                              const std::string & tag_param_name)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return KokkosCoupledVectorTagNodalValuesByName(var_name, tag_name);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledValue(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledValues(const std::string & var_name)
{
  return KokkosCoupledVectorTagValuesByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledGradient(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagGradientByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledGradients(const std::string & var_name)
{
  return KokkosCoupledVectorTagGradientsByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledNodalValue(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledNodalValues(const std::string & var_name)
{
  return KokkosCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledValueOld(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledValuesOld(const std::string & var_name)
{
  return KokkosCoupledVectorTagValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledGradientOld(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagGradientByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledGradientsOld(const std::string & var_name)
{
  return KokkosCoupledVectorTagGradientsByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledNodalValueOld(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagNodalValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledNodalValuesOld(const std::string & var_name)
{
  return KokkosCoupledVectorTagNodalValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledValueOlder(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledValuesOlder(const std::string & var_name)
{
  return KokkosCoupledVectorTagValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagGradientByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosCoupledGradientsOlder(const std::string & var_name)
{
  return KokkosCoupledVectorTagGradientsByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledNodalValueOlder(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagNodalValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledNodalValuesOlder(const std::string & var_name)
{
  return KokkosCoupledVectorTagNodalValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledDot(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

Moose::Kokkos::VariableValue
Coupleable::KokkosCoupledDots(const std::string & var_name)
{
  return KokkosCoupledVectorTagValuesByName(var_name, Moose::SOLUTION_DOT_TAG);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledNodalDot(const std::string & var_name, unsigned int comp)
{
  return KokkosCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosCoupledNodalDots(const std::string & var_name)
{
  return KokkosCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_DOT_TAG);
}

Moose::Kokkos::Scalar<const Real>
Coupleable::KokkosCoupledDotDu(const std::string & var_name, unsigned int comp)
{
  const auto var = getVar(var_name, comp);

  if (!var)
    return Moose::Kokkos::Scalar<const Real>(_kokkos_default_value_zero);
  else
    return Moose::Kokkos::Scalar<const Real>(var->sys().duDotDu(var->number() + comp));
}

Moose::Kokkos::VariableValue
Coupleable::KokkosZeroValue()
{
  auto variable = KokkosZeroVariable();

  return Moose::Kokkos::VariableValue(variable);
}

Moose::Kokkos::VariableGradient
Coupleable::KokkosZeroGradient()
{
  auto variable = KokkosZeroVariable();

  return Moose::Kokkos::VariableGradient(variable);
}

Moose::Kokkos::VariableNodalValue
Coupleable::KokkosZeroNodalValue()
{
  auto variable = KokkosZeroVariable();

  return Moose::Kokkos::VariableNodalValue(_c_fe_problem.getKokkosSystems(), variable);
}
