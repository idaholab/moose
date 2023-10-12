//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "CompileTimeDerivatives.h"
#include "DerivativeMaterialInterface.h"
#include "CompileTimeDerivativesMaterialInternal.h"

/**
 * @brief Material class to set up an expression and its derivatives built at compile time using the
 * CompileTimeDerivatives framework.
 * @tparam N number of coupled variables
 * @tparam is_ad whether to use forward mode automatic differentiation and to generate Moose AD
 * material properties
 * @tparam MaxD maximum derivative order to build
 */
template <int N, bool is_ad = false, int MaxD = 3>
class CompileTimeDerivativesMaterial : public DerivativeMaterialInterface<Material>
{
public:
  CompileTimeDerivativesMaterial(const InputParameters & params,
                                 const std::array<std::string, N> variables);
  static InputParameters validParams();

protected:
  /**
   * Entry point for the compile time loop that evaluates all derivatives and assigns them to their
   * respective  material properties
   * @arg F the compile time derivative expression object
   */
  template <typename T>
  void evaluate(const T & F);

  const MaterialPropertyName _F_name;
  GenericMaterialProperty<Real, is_ad> & _prop_F;

  /**
   * The names of the coupled variables
   */
  std::array<VariableName, N> _var_name;

  /**
   * A serialized list of material properties for all derivatives up to and including order MaxD
   */
  std::array<GenericMaterialProperty<Real, is_ad> *,
             CompileTimeDerivativesMaterialInternal::total_derivatives<MaxD, N>()>
      _prop_dF;

  /**
   * Set up a tuple with one entry per coupled variable (each entry with an increment). The coupled
   * variables are wrapped in a CTD reference object with a different tag for differentiation. This
   * makes each reference a different type, so we need to store them in a tuple (as opposed to an
   * array).
   */
  typename decltype(CompileTimeDerivativesMaterialInternal::make_tuple_array<
                    GenericVariableValue<is_ad>>(std::make_index_sequence<N>{}))::type _refs;

private:
  template <std::size_t... Ns>
  auto makeRefsTuple(const std::array<std::string, N> & variables,
                     const unsigned int & qp,
                     std::index_sequence<Ns...>)
  {
    return std::make_tuple(
        CompileTimeDerivatives::makeRef<Ns>(coupledGenericValue<is_ad>(variables[Ns]), qp)...);
  }

  template <std::size_t I>
  constexpr void loopDeclareProperties(std::index_sequence<>)
  {
  }
  template <std::size_t I, std::size_t first, std::size_t... tail>
  constexpr void loopDeclareProperties(std::index_sequence<first, tail...> int_seq)
  {
    using namespace CompileTimeDerivativesMaterialInternal;
    if constexpr (is_sorted(int_seq))
    {
      _prop_dF[I] = &declarePropertyDerivative<Real, is_ad>(
          _F_name, std::vector<VariableName>{_var_name[first], _var_name[tail]...});
      loopDeclareProperties<I + 1>(details::increment<N>(int_seq));
    }
    else
      loopDeclareProperties<I>(details::increment<N>(int_seq));
  }

  /**
   * Entry point for the compile time loop that declares all derivative material properties
   */
  void declareProperties()
  {
    using namespace CompileTimeDerivativesMaterialInternal;
    loopDeclareProperties<0>(zeroes<MaxD>());
  }

  template <std::size_t I, typename T>
  constexpr void loopEvaluate(const T &, std::index_sequence<>)
  {
  }
  template <std::size_t I, typename T, std::size_t first, std::size_t... tail>
  constexpr void loopEvaluate(const T & expression, std::index_sequence<first, tail...> int_seq)
  {
    using namespace CompileTimeDerivativesMaterialInternal;
    if constexpr (is_sorted(int_seq))
    {
      (*_prop_dF[I])[_qp] = take_derivatives(expression, int_seq)();
      loopEvaluate<I + 1>(expression, details::increment<N>(int_seq));
    }
    else
      loopEvaluate<I>(expression, details::increment<N>(int_seq));
  }
};

template <int N, bool is_ad, int MaxD>
InputParameters
CompileTimeDerivativesMaterial<N, is_ad, MaxD>::validParams()
{
  auto params = DerivativeMaterialInterface<Material>::validParams();
  params.addRequiredParam<MaterialPropertyName>("property_name",
                                                "Name of the parsed material property");
  return params;
}

template <int N, bool is_ad, int MaxD>
CompileTimeDerivativesMaterial<N, is_ad, MaxD>::CompileTimeDerivativesMaterial(
    const InputParameters & params, const std::array<std::string, N> variables)
  : DerivativeMaterialInterface<Material>(params),
    _F_name(getParam<MaterialPropertyName>("property_name")),
    _prop_F(declareGenericProperty<Real, is_ad>(_F_name)),
    _refs(makeRefsTuple(variables, _qp, std::make_index_sequence<N>{}))
{
  // get the names of all coupled variables
  for (const auto i : make_range(N))
    _var_name[i] = coupledName(variables[i]);

  // declare all material properties
  declareProperties();
}

template <int N, bool is_ad, int MaxD>
template <typename T>
void
CompileTimeDerivativesMaterial<N, is_ad, MaxD>::evaluate(const T & F)
{
  _prop_F[_qp] = F();
  using namespace CompileTimeDerivativesMaterialInternal;
  loopEvaluate<0>(F, zeroes<MaxD>());
}
