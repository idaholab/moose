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

namespace
{
template <typename T, typename Func>
Moose::Kokkos::Array<T>
kokkosCoupledArrayHelper(const unsigned int components, const Func & func)
{
  Moose::Kokkos::Array<T> values(components);

  for (const auto i : make_range(components))
    values[i] = func(i);

  values.copyToDevice();

  return values;
}

// The time derivative of the constant default value of an uncoupled variable is zero
bool
isTimeDerivativeTag(const std::string & tag_name)
{
  return tag_name == Moose::SOLUTION_DOT_TAG || tag_name == Moose::SOLUTION_DOTDOT_TAG;
}

void
checkNodal(const Moose::Kokkos::Variable & variable,
           const std::string & var_name,
           const std::string & kind)
{
  if (variable.coupled() && !variable.nodal())
    mooseError("Cannot get nodal values from the coupled ",
               kind,
               " '",
               var_name,
               "', because the associated variable is not nodal.");
}
}

TagID
Coupleable::kokkosRequestCoupledVectorTag(const std::string & var_name,
                                          const std::string & tag_name,
                                          const std::vector<MooseVariableFieldBase *> & vars) const
{
  const bool is_old = tag_name == Moose::OLD_SOLUTION_TAG || tag_name == Moose::OLDER_SOLUTION_TAG;

  for (auto var : vars)
  {
    var->requireQpComputations();

    if (tag_name == Moose::OLD_SOLUTION_TAG)
      var->sys().needSolutionState(1);
    else if (tag_name == Moose::OLDER_SOLUTION_TAG)
      var->sys().needSolutionState(2);
  }

  coupledCallback(var_name, is_old);

  const auto tag = _c_fe_problem.getVectorTagID(tag_name);

  const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

  return tag;
}

const TagName &
Coupleable::kokkosTagName(const std::string & tag_param_name) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  return _c_parameters.get<TagName>(tag_param_name);
}

Moose::Kokkos::Variable
Coupleable::kokkosCoupledVectorTagVariable(const std::string & var_name,
                                           const std::string & tag_name,
                                           unsigned int comp,
                                           bool zero_default) const
{
  zero_default = zero_default || isTimeDerivativeTag(tag_name);

  Moose::Kokkos::Variable variable;

  if (isCoupled(var_name))
  {
    auto var = const_cast<MooseVariableField<Real> *>(
        getVarHelper<MooseVariableField<Real>>(var_name, comp));

    variable.init(*var, kokkosRequestCoupledVectorTag(var_name, tag_name, {var}));
  }
  else
  {
    std::vector<Real> default_value = {
        zero_default ? 0 : _c_parameters.defaultCoupledValue(var_name, comp)};

    variable.init(default_value, CoupleableKey{});
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::kokkosCoupledVectorTagVariables(const std::string & var_name,
                                            const std::string & tag_name,
                                            bool zero_default) const
{
  zero_default = zero_default || isTimeDerivativeTag(tag_name);

  Moose::Kokkos::Variable variable;

  const auto components = coupledComponents(var_name);

  if (isCoupled(var_name))
  {
    std::vector<MooseVariableFieldBase *> vars;

    for (unsigned int comp = 0; comp < components; ++comp)
      vars.push_back(const_cast<MooseVariableField<Real> *>(
          getVarHelper<MooseVariableField<Real>>(var_name, comp)));

    variable.init(vars, kokkosRequestCoupledVectorTag(var_name, tag_name, vars));
  }
  else
  {
    std::vector<Real> default_values(components, 0);

    if (!zero_default)
      for (unsigned int comp = 0; comp < components; ++comp)
        default_values[comp] = _c_parameters.defaultCoupledValue(var_name, comp);

    variable.init(default_values, CoupleableKey{});
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::kokkosCoupledVectorTagVectorVariable(const std::string & var_name,
                                                 const std::string & tag_name,
                                                 unsigned int comp,
                                                 bool zero_default) const
{
  zero_default = zero_default || isTimeDerivativeTag(tag_name);

  Moose::Kokkos::Variable variable;

  if (isCoupled(var_name))
  {
    auto var = const_cast<MooseVariableField<RealVectorValue> *>(
        getVarHelper<MooseVariableField<RealVectorValue>>(var_name, comp));

    variable.init(*var, kokkosRequestCoupledVectorTag(var_name, tag_name, {var}));
  }
  else
  {
    Moose::Kokkos::Real3 default_value = 0;

    if (!zero_default)
      default_value = (*getDefaultVectorValue(var_name))[0];

    variable.init({default_value}, CoupleableKey{});
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::kokkosCoupledVectorTagVectorVariables(const std::string & var_name,
                                                  const std::string & tag_name,
                                                  bool zero_default) const
{
  zero_default = zero_default || isTimeDerivativeTag(tag_name);

  Moose::Kokkos::Variable variable;

  const auto components = coupledComponents(var_name);

  if (isCoupled(var_name))
  {
    std::vector<MooseVariableFieldBase *> vars;

    for (unsigned int comp = 0; comp < components; ++comp)
      vars.push_back(const_cast<MooseVariableField<RealVectorValue> *>(
          getVarHelper<MooseVariableField<RealVectorValue>>(var_name, comp)));

    variable.init(vars, kokkosRequestCoupledVectorTag(var_name, tag_name, vars));
  }
  else
  {
    Moose::Kokkos::Real3 default_value = 0;

    if (!zero_default)
      default_value = (*getDefaultVectorValue(var_name))[0];

    variable.init({default_value}, CoupleableKey{});
  }

  return variable;
}

Moose::Kokkos::Variable
Coupleable::kokkosCoupledVectorTagArrayVariable(const std::string & var_name,
                                                const std::string & tag_name,
                                                unsigned int comp,
                                                bool zero_default) const
{
  zero_default = zero_default || isTimeDerivativeTag(tag_name);

  Moose::Kokkos::Variable variable;

  if (isCoupled(var_name))
  {
    auto var = const_cast<ArrayMooseVariable *>(getArrayVar(var_name, comp));

    variable.init(*var, kokkosRequestCoupledVectorTag(var_name, tag_name, {var}));
  }
  else
  {
    // An uncoupled array variable takes one default value per array component
    const auto n = _c_parameters.numberDefaultCoupledValues(var_name);
    std::vector<Real> default_values(n, 0);

    if (!zero_default)
      for (const auto i : make_range(n))
        default_values[i] = _c_parameters.defaultCoupledValue(var_name, i);

    variable.init(default_values, CoupleableKey{});
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

/*
 * The coupling accessors below differ only in their variable family, Kokkos wrapper type, vector
 * tag, and solution state, so they are generated with the following macros. Each macro defines the
 * single-component accessor Name and the multi-component accessor that appends "s" to Name.
 */

/**
 * Define the accessors Name##ByName that wrap the Kokkos variable built by Builder##Variable(s) for
 * a vector tag name, where Wrap is the statement that returns the wrapper of `variable`, and
 * ZeroDefault is whether an uncoupled variable takes zero instead of its default value
 */
#define KOKKOS_COUPLED_BY_NAME(Ret, Name, Builder, ZeroDefault, Wrap)                              \
  Ret Coupleable::Name##ByName(                                                                    \
      const std::string & var_name, const std::string & tag_name, unsigned int comp) const         \
  {                                                                                                \
    auto variable = Builder##Variable(var_name, tag_name, comp, ZeroDefault);                      \
                                                                                                   \
    Wrap;                                                                                          \
  }                                                                                                \
                                                                                                   \
  Ret Coupleable::Name##s##ByName(const std::string & var_name, const std::string & tag_name)      \
      const                                                                                        \
  {                                                                                                \
    auto variable = Builder##Variables(var_name, tag_name, ZeroDefault);                           \
                                                                                                   \
    Wrap;                                                                                          \
  }

/**
 * Define the accessors Name##ByName for an array variable, where the multi-component accessor
 * returns one wrapper per coupled array variable
 */
#define KOKKOS_COUPLED_ARRAY_BY_NAME(Ret, Name, ZeroDefault, Wrap)                                 \
  Ret Coupleable::Name##ByName(                                                                    \
      const std::string & var_name, const std::string & tag_name, unsigned int comp) const         \
  {                                                                                                \
    auto variable = kokkosCoupledVectorTagArrayVariable(var_name, tag_name, comp, ZeroDefault);    \
                                                                                                   \
    Wrap;                                                                                          \
  }                                                                                                \
                                                                                                   \
  Moose::Kokkos::Array<Ret> Coupleable::Name##s##ByName(const std::string & var_name,              \
                                                        const std::string & tag_name) const        \
  {                                                                                                \
    return kokkosCoupledArrayHelper<Ret>(coupledComponents(var_name),                              \
                                         [&](const unsigned int i)                                 \
                                         { return Name##ByName(var_name, tag_name, i); });         \
  }

/**
 * Define the accessors Name that take the vector tag name from the parameter tag_param_name and
 * forward to Name##ByName
 */
#define KOKKOS_COUPLED_TAG_PARAM(Ret, RetMulti, Name)                                              \
  Ret Coupleable::Name(                                                                            \
      const std::string & var_name, const std::string & tag_param_name, unsigned int comp) const   \
  {                                                                                                \
    return Name##ByName(var_name, kokkosTagName(tag_param_name), comp);                            \
  }                                                                                                \
                                                                                                   \
  RetMulti Coupleable::Name##s(const std::string & var_name, const std::string & tag_param_name)   \
      const                                                                                        \
  {                                                                                                \
    return Name##s##ByName(var_name, kokkosTagName(tag_param_name));                               \
  }

/**
 * Define the accessors Name##State that forward to Forward##ByName with the vector tag Tag
 */
#define KOKKOS_COUPLED_STATE(Ret, RetMulti, Name, State, Tag, Forward)                             \
  Ret Coupleable::Name##State(const std::string & var_name, unsigned int comp) const               \
  {                                                                                                \
    return Forward##ByName(var_name, Tag, comp);                                                   \
  }                                                                                                \
                                                                                                   \
  RetMulti Coupleable::Name##s##State(const std::string & var_name) const                          \
  {                                                                                                \
    return Forward##s##ByName(var_name, Tag);                                                      \
  }

/**
 * Define the accessors Name##State that forward to Nodal##ByName for nodal objects and to
 * Forward##ByName otherwise, with the vector tag Tag
 */
#define KOKKOS_COUPLED_NODAL_STATE(Ret, RetMulti, Name, State, Tag, Forward, Nodal)                \
  Ret Coupleable::Name##State(const std::string & var_name, unsigned int comp) const               \
  {                                                                                                \
    return _c_nodal ? Nodal##ByName(var_name, Tag, comp) : Forward##ByName(var_name, Tag, comp);   \
  }                                                                                                \
                                                                                                   \
  RetMulti Coupleable::Name##s##State(const std::string & var_name) const                          \
  {                                                                                                \
    return _c_nodal ? Nodal##s##ByName(var_name, Tag) : Forward##s##ByName(var_name, Tag);         \
  }

/**
 * Define all accessors of a variable family that take a vector tag parameter or a solution state
 * @param AD Empty, or AD for the automatic differentiation accessors
 * @param F Empty, Vector, or Array for the variable family
 * @param Val The value wrapper type
 * @param Grad The gradient wrapper type
 * @param Vals The multi-component value wrapper type
 * @param Grads The multi-component gradient wrapper type
 */
#define KOKKOS_COUPLED_FAMILY(AD, F, Val, Grad, Vals, Grads)                                       \
  KOKKOS_COUPLED_TAG_PARAM(Val, Vals, kokkos##AD##CoupledVectorTag##F##Value)                      \
  KOKKOS_COUPLED_TAG_PARAM(Grad, Grads, kokkos##AD##CoupledVectorTag##F##Gradient)                 \
  KOKKOS_COUPLED_TAG_PARAM(Val, Vals, kokkos##AD##CoupledVectorTag##F##NodalValue)                 \
  KOKKOS_COUPLED_TAG_PARAM(Val, Vals, kokkos##AD##CoupledVectorTag##F##DofValue)                   \
  KOKKOS_COUPLED_FAMILY_STATE(AD, F, Val, Grad, Vals, Grads, , Moose::SOLUTION_TAG)                \
  KOKKOS_COUPLED_FAMILY_STATE(AD, F, Val, Grad, Vals, Grads, Old, Moose::OLD_SOLUTION_TAG)         \
  KOKKOS_COUPLED_FAMILY_STATE(AD, F, Val, Grad, Vals, Grads, Older, Moose::OLDER_SOLUTION_TAG)     \
  KOKKOS_COUPLED_NODAL_STATE(Val,                                                                  \
                             Vals,                                                                 \
                             kokkos##AD##Coupled##F##Dot,                                          \
                             ,                                                                     \
                             Moose::SOLUTION_DOT_TAG,                                              \
                             kokkos##AD##CoupledVectorTag##F##Value,                               \
                             kokkos##AD##CoupledVectorTag##F##NodalValue)                          \
  KOKKOS_COUPLED_STATE(Val,                                                                        \
                       Vals,                                                                       \
                       kokkos##AD##Coupled##F##NodalDot,                                           \
                       ,                                                                           \
                       Moose::SOLUTION_DOT_TAG,                                                    \
                       kokkos##AD##CoupledVectorTag##F##NodalValue)

/**
 * Define the value, gradient, nodal value, and DOF value accessors of a variable family for the
 * solution state State with the vector tag Tag
 */
#define KOKKOS_COUPLED_FAMILY_STATE(AD, F, Val, Grad, Vals, Grads, State, Tag)                     \
  KOKKOS_COUPLED_NODAL_STATE(Val,                                                                  \
                             Vals,                                                                 \
                             kokkos##AD##Coupled##F##Value,                                        \
                             State,                                                                \
                             Tag,                                                                  \
                             kokkos##AD##CoupledVectorTag##F##Value,                               \
                             kokkos##AD##CoupledVectorTag##F##NodalValue)                          \
  KOKKOS_COUPLED_STATE(Grad,                                                                       \
                       Grads,                                                                      \
                       kokkos##AD##Coupled##F##Gradient,                                           \
                       State,                                                                      \
                       Tag,                                                                        \
                       kokkos##AD##CoupledVectorTag##F##Gradient)                                  \
  KOKKOS_COUPLED_STATE(Val,                                                                        \
                       Vals,                                                                       \
                       kokkos##AD##Coupled##F##NodalValue,                                         \
                       State,                                                                      \
                       Tag,                                                                        \
                       kokkos##AD##CoupledVectorTag##F##NodalValue)                                \
  KOKKOS_COUPLED_STATE(Val,                                                                        \
                       Vals,                                                                       \
                       kokkos##AD##Coupled##F##DofValue,                                           \
                       State,                                                                      \
                       Tag,                                                                        \
                       kokkos##AD##CoupledVectorTag##F##DofValue)

// Standard variables
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::VariableValue,
                       kokkosCoupledVectorTagValue,
                       kokkosCoupledVectorTag,
                       false,
                       return Moose::Kokkos::VariableValue(variable))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::VariableValue,
                       kokkosCoupledVectorTagNodalValue,
                       kokkosCoupledVectorTag,
                       false,
                       checkNodal(variable, var_name, "variable");
                       return Moose::Kokkos::VariableValue(variable, true))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::VariableValue,
                       kokkosCoupledVectorTagDofValue,
                       kokkosCoupledVectorTag,
                       false,
                       return Moose::Kokkos::VariableValue(variable, true))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::VariableGradient,
                       kokkosCoupledVectorTagGradient,
                       kokkosCoupledVectorTag,
                       true,
                       return Moose::Kokkos::VariableGradient(variable))

KOKKOS_COUPLED_FAMILY(,
                      ,
                      Moose::Kokkos::VariableValue,
                      Moose::Kokkos::VariableGradient,
                      Moose::Kokkos::VariableValue,
                      Moose::Kokkos::VariableGradient)

// Vector variables
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::VectorVariableValue,
                       kokkosCoupledVectorTagVectorValue,
                       kokkosCoupledVectorTagVector,
                       false,
                       return Moose::Kokkos::VectorVariableValue(variable))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::VectorVariableGradient,
                       kokkosCoupledVectorTagVectorGradient,
                       kokkosCoupledVectorTagVector,
                       true,
                       return Moose::Kokkos::VectorVariableGradient(variable))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::VectorVariableValue,
                       kokkosCoupledVectorTagVectorDofValue,
                       kokkosCoupledVectorTagVector,
                       false,
                       return Moose::Kokkos::VectorVariableValue(variable, true))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::VectorVariableValue,
                       kokkosCoupledVectorTagVectorNodalValue,
                       kokkosCoupledVectorTagVector,
                       false,
                       checkNodal(variable, var_name, "vector variable");
                       return Moose::Kokkos::VectorVariableValue(variable, true))

KOKKOS_COUPLED_FAMILY(,
                      Vector,
                      Moose::Kokkos::VectorVariableValue,
                      Moose::Kokkos::VectorVariableGradient,
                      Moose::Kokkos::VectorVariableValue,
                      Moose::Kokkos::VectorVariableGradient)

// Array variables
KOKKOS_COUPLED_ARRAY_BY_NAME(Moose::Kokkos::ArrayVariableValue,
                             kokkosCoupledVectorTagArrayValue,
                             false,
                             return Moose::Kokkos::ArrayVariableValue(variable))
KOKKOS_COUPLED_ARRAY_BY_NAME(Moose::Kokkos::ArrayVariableGradient,
                             kokkosCoupledVectorTagArrayGradient,
                             true,
                             return Moose::Kokkos::ArrayVariableGradient(variable))
KOKKOS_COUPLED_ARRAY_BY_NAME(Moose::Kokkos::ArrayVariableValue,
                             kokkosCoupledVectorTagArrayNodalValue,
                             false,
                             checkNodal(variable, var_name, "array variable");
                             return Moose::Kokkos::ArrayVariableValue(variable, true))
KOKKOS_COUPLED_ARRAY_BY_NAME(Moose::Kokkos::ArrayVariableValue,
                             kokkosCoupledVectorTagArrayDofValue,
                             false,
                             return Moose::Kokkos::ArrayVariableValue(variable, true))

KOKKOS_COUPLED_FAMILY(,
                      Array,
                      Moose::Kokkos::ArrayVariableValue,
                      Moose::Kokkos::ArrayVariableGradient,
                      Moose::Kokkos::Array<Moose::Kokkos::ArrayVariableValue>,
                      Moose::Kokkos::Array<Moose::Kokkos::ArrayVariableGradient>)

// Automatic differentiation variables
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::ADVariableValue,
                       kokkosADCoupledVectorTagValue,
                       kokkosCoupledVectorTag,
                       false,
                       return Moose::Kokkos::ADVariableValue(variable))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::ADVariableGradient,
                       kokkosADCoupledVectorTagGradient,
                       kokkosCoupledVectorTag,
                       true,
                       return Moose::Kokkos::ADVariableGradient(variable))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::ADVariableValue,
                       kokkosADCoupledVectorTagNodalValue,
                       kokkosCoupledVectorTag,
                       false,
                       checkNodal(variable, var_name, "variable");
                       return Moose::Kokkos::ADVariableValue(variable, true))
KOKKOS_COUPLED_BY_NAME(Moose::Kokkos::ADVariableValue,
                       kokkosADCoupledVectorTagDofValue,
                       kokkosCoupledVectorTag,
                       false,
                       return Moose::Kokkos::ADVariableValue(variable, true))

KOKKOS_COUPLED_FAMILY(AD,
                      ,
                      Moose::Kokkos::ADVariableValue,
                      Moose::Kokkos::ADVariableGradient,
                      Moose::Kokkos::ADVariableValue,
                      Moose::Kokkos::ADVariableGradient)

#undef KOKKOS_COUPLED_BY_NAME
#undef KOKKOS_COUPLED_ARRAY_BY_NAME
#undef KOKKOS_COUPLED_TAG_PARAM
#undef KOKKOS_COUPLED_STATE
#undef KOKKOS_COUPLED_NODAL_STATE
#undef KOKKOS_COUPLED_FAMILY
#undef KOKKOS_COUPLED_FAMILY_STATE

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
