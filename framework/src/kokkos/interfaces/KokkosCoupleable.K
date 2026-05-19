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

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagValueByName(const std::string & var_name,
                                                const std::string & tag_name,
                                                unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::ADVariableValue(variable);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagValuesByName(const std::string & var_name,
                                                 const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::ADVariableValue(variable);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledVectorTagGradientByName(const std::string & var_name,
                                                   const std::string & tag_name,
                                                   unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::ADVariableGradient(variable);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledVectorTagGradientsByName(const std::string & var_name,
                                                    const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::ADVariableGradient(variable);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagNodalValueByName(const std::string & var_name,
                                                     const std::string & tag_name,
                                                     unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  if (!variable.nodal())
    mooseError("Cannot get nodal values from the coupled variable '",
               var_name,
               "', because the associated variable is not nodal.");

  return Moose::Kokkos::ADVariableValue(variable, true);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagNodalValuesByName(const std::string & var_name,
                                                      const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  if (!variable.nodal())
    mooseError("Cannot get nodal values from the coupled variable '",
               var_name,
               "', because the associated variable is not nodal.");

  return Moose::Kokkos::ADVariableValue(variable, true);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagDofValueByName(const std::string & var_name,
                                                   const std::string & tag_name,
                                                   unsigned int comp) const
{
  auto variable = kokkosCoupledVectorTagVariable(var_name, tag_name, comp);

  return Moose::Kokkos::ADVariableValue(variable, true);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagDofValuesByName(const std::string & var_name,
                                                    const std::string & tag_name) const
{
  auto variable = kokkosCoupledVectorTagVariables(var_name, tag_name);

  return Moose::Kokkos::ADVariableValue(variable, true);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagValue(const std::string & var_name,
                                          const std::string & tag_param_name,
                                          unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosADCoupledVectorTagValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagValues(const std::string & var_name,
                                           const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosADCoupledVectorTagValuesByName(var_name, tag_name);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledVectorTagGradient(const std::string & var_name,
                                             const std::string & tag_param_name,
                                             unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosADCoupledVectorTagGradientByName(var_name, tag_name, comp);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledVectorTagGradients(const std::string & var_name,
                                              const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosADCoupledVectorTagGradientsByName(var_name, tag_name);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagNodalValue(const std::string & var_name,
                                               const std::string & tag_param_name,
                                               unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosADCoupledVectorTagNodalValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagNodalValues(const std::string & var_name,
                                                const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosADCoupledVectorTagNodalValuesByName(var_name, tag_name);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagDofValue(const std::string & var_name,
                                             const std::string & tag_param_name,
                                             unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosADCoupledVectorTagDofValueByName(var_name, tag_name, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledVectorTagDofValues(const std::string & var_name,
                                              const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  TagName tag_name = _c_parameters.get<TagName>(tag_param_name);

  return kokkosADCoupledVectorTagDofValuesByName(var_name, tag_name);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledValue(const std::string & var_name, unsigned int comp) const
{
  return _c_nodal ? kokkosADCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_TAG, comp)
                  : kokkosADCoupledVectorTagValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledValues(const std::string & var_name) const
{
  return _c_nodal ? kokkosADCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_TAG)
                  : kokkosADCoupledVectorTagValuesByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledGradient(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagGradientByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledGradients(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagGradientsByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledNodalValue(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledNodalValues(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledDofValue(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagDofValueByName(var_name, Moose::SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledDofValues(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagDofValuesByName(var_name, Moose::SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledValueOld(const std::string & var_name, unsigned int comp) const
{
  return _c_nodal
             ? kokkosADCoupledVectorTagNodalValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp)
             : kokkosADCoupledVectorTagValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledValuesOld(const std::string & var_name) const
{
  return _c_nodal ? kokkosADCoupledVectorTagNodalValuesByName(var_name, Moose::OLD_SOLUTION_TAG)
                  : kokkosADCoupledVectorTagValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledGradientOld(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagGradientByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledGradientsOld(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagGradientsByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledNodalValueOld(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagNodalValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledNodalValuesOld(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagNodalValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledDofValueOld(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagDofValueByName(var_name, Moose::OLD_SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledDofValuesOld(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagDofValuesByName(var_name, Moose::OLD_SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledValueOlder(const std::string & var_name, unsigned int comp) const
{
  return _c_nodal
             ? kokkosADCoupledVectorTagNodalValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp)
             : kokkosADCoupledVectorTagValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledValuesOlder(const std::string & var_name) const
{
  return _c_nodal ? kokkosADCoupledVectorTagNodalValuesByName(var_name, Moose::OLDER_SOLUTION_TAG)
                  : kokkosADCoupledVectorTagValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledGradientOlder(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagGradientByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableGradient
Coupleable::kokkosADCoupledGradientsOlder(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagGradientsByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledNodalValueOlder(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagNodalValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledNodalValuesOlder(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagNodalValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledDofValueOlder(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagDofValueByName(var_name, Moose::OLDER_SOLUTION_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledDofValuesOlder(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagDofValuesByName(var_name, Moose::OLDER_SOLUTION_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledDot(const std::string & var_name, unsigned int comp) const
{
  return _c_nodal
             ? kokkosADCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp)
             : kokkosADCoupledVectorTagValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledDots(const std::string & var_name) const
{
  return _c_nodal ? kokkosADCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_DOT_TAG)
                  : kokkosADCoupledVectorTagValuesByName(var_name, Moose::SOLUTION_DOT_TAG);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledNodalDot(const std::string & var_name, unsigned int comp) const
{
  return kokkosADCoupledVectorTagNodalValueByName(var_name, Moose::SOLUTION_DOT_TAG, comp);
}

Moose::Kokkos::ADVariableValue
Coupleable::kokkosADCoupledNodalDots(const std::string & var_name) const
{
  return kokkosADCoupledVectorTagNodalValuesByName(var_name, Moose::SOLUTION_DOT_TAG);
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
