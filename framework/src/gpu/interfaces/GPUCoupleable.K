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

GPUVariable
Coupleable::GPUCoupledVectorTagVariable(const std::string & var_name,
                                        const std::string & tag_name,
                                        unsigned int comp)
{
  GPUVariable variable;

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

GPUVariable
Coupleable::GPUCoupledVectorTagVariables(const std::string & var_name, const std::string & tag_name)
{
  GPUVariable variable;

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

GPUVariable
Coupleable::GPUZeroVariable()
{
  GPUVariable variable;

  variable._default_value.create(1);
  variable._default_value[0] = 0;
  variable._default_value.copy();

  return variable;
}

GPUVariableValue
Coupleable::GPUCoupledVectorTagValueByName(const std::string & var_name,
                                           const std::string & tag_name,
                                           unsigned int comp)
{
  GPUVariable variable = GPUCoupledVectorTagVariable(var_name, tag_name, comp);

  return GPUVariableValue(_c_fe_problem.getGPUSystems(), variable);
}

GPUVariableValue
Coupleable::GPUCoupledVectorTagValuesByName(const std::string & var_name,
                                            const std::string & tag_name)
{
  GPUVariable variable = GPUCoupledVectorTagVariables(var_name, tag_name);

  return GPUVariableValue(_c_fe_problem.getGPUSystems(), variable);
}

GPUVariableGradient
Coupleable::GPUCoupledVectorTagGradientByName(const std::string & var_name,
                                              const std::string & tag_name,
                                              unsigned int comp)
{
  GPUVariable variable = GPUCoupledVectorTagVariable(var_name, tag_name, comp);

  return GPUVariableGradient(_c_fe_problem.getGPUSystems(), variable);
}

GPUVariableGradient
Coupleable::GPUCoupledVectorTagGradientsByName(const std::string & var_name,
                                               const std::string & tag_name)
{
  GPUVariable variable = GPUCoupledVectorTagVariables(var_name, tag_name);

  return GPUVariableGradient(_c_fe_problem.getGPUSystems(), variable);
}

GPUVariableNodalValue
Coupleable::GPUCoupledVectorTagNodalValueByName(const std::string & var_name,
                                                const std::string & tag_name,
                                                unsigned int comp)
{
  GPUVariable variable = GPUCoupledVectorTagVariable(var_name, tag_name, comp);

  return GPUVariableNodalValue(_c_fe_problem.getGPUSystems(), variable);
}

GPUVariableNodalValue
Coupleable::GPUCoupledVectorTagNodalValuesByName(const std::string & var_name,
                                                 const std::string & tag_name)
{
  GPUVariable variable = GPUCoupledVectorTagVariables(var_name, tag_name);

  return GPUVariableNodalValue(_c_fe_problem.getGPUSystems(), variable);
}

GPUVariableValue
Coupleable::GPUCoupledVectorTagValue(const std::string & var_name,
                                     const std::string & tag_param_name,
                                     unsigned int comp)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return GPUCoupledVectorTagValueByName(var_name, tag_name, comp);
}

GPUVariableValue
Coupleable::GPUCoupledVectorTagValues(const std::string & var_name,
                                      const std::string & tag_param_name)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return GPUCoupledVectorTagValuesByName(var_name, tag_name);
}

GPUVariableGradient
Coupleable::GPUCoupledVectorTagGradient(const std::string & var_name,
                                        const std::string & tag_param_name,
                                        unsigned int comp)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return GPUCoupledVectorTagGradientByName(var_name, tag_name, comp);
}

GPUVariableGradient
Coupleable::GPUCoupledVectorTagGradients(const std::string & var_name,
                                         const std::string & tag_param_name)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return GPUCoupledVectorTagGradientsByName(var_name, tag_name);
}

GPUVariableNodalValue
Coupleable::GPUCoupledVectorTagNodalValue(const std::string & var_name,
                                          const std::string & tag_param_name,
                                          unsigned int comp)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return GPUCoupledVectorTagNodalValueByName(var_name, tag_name, comp);
}

GPUVariableNodalValue
Coupleable::GPUCoupledVectorTagNodalValues(const std::string & var_name,
                                           const std::string & tag_param_name)
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return GPUCoupledVectorTagNodalValuesByName(var_name, tag_name);
}

GPUVariableValue
Coupleable::GPUCoupledValue(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

GPUVariableValue
Coupleable::GPUCoupledValues(const std::string & var_name)
{
  return GPUCoupledVectorTagValuesByName(var_name, Moose::SOLUTION_TAG);
}

GPUVariableGradient
Coupleable::GPUCoupledGradient(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagGradientByName(var_name, Moose::SOLUTION_TAG, comp);
}

GPUVariableGradient
Coupleable::GPUCoupledGradients(const std::string & var_name)
{
  return GPUCoupledVectorTagGradientsByName(var_name, Moose::SOLUTION_TAG);
}

GPUVariableNodalValue
Coupleable::GPUCoupledNodalValue(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

GPUVariableNodalValue
Coupleable::GPUCoupledNodalValues(const std::string & var_name)
{
  return GPUCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_TAG);
}

GPUVariableValue
Coupleable::GPUCoupledValueOld(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

GPUVariableValue
Coupleable::GPUCoupledValuesOld(const std::string & var_name)
{
  return GPUCoupledVectorTagValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

GPUVariableGradient
Coupleable::GPUCoupledGradientOld(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagGradientByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

GPUVariableGradient
Coupleable::GPUCoupledGradientsOld(const std::string & var_name)
{
  return GPUCoupledVectorTagGradientsByName(var_name, Moose::OLD_SOLUTION_TAG);
}

GPUVariableNodalValue
Coupleable::GPUCoupledNodalValueOld(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagNodalValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

GPUVariableNodalValue
Coupleable::GPUCoupledNodalValuesOld(const std::string & var_name)
{
  return GPUCoupledVectorTagNodalValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

GPUVariableValue
Coupleable::GPUCoupledValueOlder(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

GPUVariableValue
Coupleable::GPUCoupledValuesOlder(const std::string & var_name)
{
  return GPUCoupledVectorTagValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

GPUVariableGradient
Coupleable::GPUCoupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagGradientByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

GPUVariableGradient
Coupleable::GPUCoupledGradientsOlder(const std::string & var_name)
{
  return GPUCoupledVectorTagGradientsByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

GPUVariableNodalValue
Coupleable::GPUCoupledNodalValueOlder(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagNodalValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

GPUVariableNodalValue
Coupleable::GPUCoupledNodalValuesOlder(const std::string & var_name)
{
  return GPUCoupledVectorTagNodalValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

GPUVariableValue
Coupleable::GPUCoupledDot(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

GPUVariableValue
Coupleable::GPUCoupledDots(const std::string & var_name)
{
  return GPUCoupledVectorTagValuesByName(var_name, Moose::SOLUTION_DOT_TAG);
}

GPUVariableNodalValue
Coupleable::GPUCoupledNodalDot(const std::string & var_name, unsigned int comp)
{
  return GPUCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

GPUVariableNodalValue
Coupleable::GPUCoupledNodalDots(const std::string & var_name)
{
  return GPUCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_DOT_TAG);
}

GPUScalar<const Real>
Coupleable::GPUCoupledDotDu(const std::string & var_name, unsigned int comp)
{
  const auto var = getVar(var_name, comp);

  if (!var)
    return GPUScalar<const Real>(_GPU_default_value_zero);
  else
    return GPUScalar<const Real>(var->sys().duDotDu(var->number() + comp));
}

GPUVariableValue
Coupleable::GPUZeroValue()
{
  GPUVariable variable = GPUZeroVariable();

  return GPUVariableValue(_c_fe_problem.getGPUSystems(), variable);
}

GPUVariableGradient
Coupleable::GPUZeroGradient()
{
  GPUVariable variable = GPUZeroVariable();

  return GPUVariableGradient(_c_fe_problem.getGPUSystems(), variable);
}

GPUVariableNodalValue
Coupleable::GPUZeroNodalValue()
{
  GPUVariable variable = GPUZeroVariable();

  return GPUVariableNodalValue(_c_fe_problem.getGPUSystems(), variable);
}
