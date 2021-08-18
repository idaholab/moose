//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParsedMaterialHelper.h"
#include "DerivativeMaterialPropertyNameInterface.h"

#include "libmesh/fparser_ad.hh"

#define usingDerivativeParsedMaterialHelperMembers(T)                                              \
  usingParsedMaterialHelperMembers(T);                                                             \
  using typename DerivativeParsedMaterialHelperTempl<T>::Derivative;                               \
  using typename DerivativeParsedMaterialHelperTempl<T>::MaterialPropertyDerivativeRule;           \
  using DerivativeParsedMaterialHelperTempl<T>::_derivative_order;                                 \
  using DerivativeParsedMaterialHelperTempl<T>::_derivatives

/**
 * Helper class to perform the auto derivative taking.
 */
template <bool is_ad>
class DerivativeParsedMaterialHelperTempl : public ParsedMaterialHelper<is_ad>
{
protected:
  usingParsedMaterialHelperMembers(is_ad);

public:
  typedef DerivativeMaterialPropertyNameInterface::SymbolName SymbolName;

  DerivativeParsedMaterialHelperTempl(
      const InputParameters & parameters,
      VariableNameMappingMode map_mode = VariableNameMappingMode::USE_PARAM_NAMES);

  static InputParameters validParams();

protected:
  struct Derivative;
  struct MaterialPropertyDerivativeRule;

  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  void functionsPostParse() override;
  void assembleDerivatives();

  void
  recurseMatProps(unsigned int var, unsigned int order, const MatPropDescriptorList & parent_mpd);

  void
  recurseDerivative(unsigned int var, unsigned int order, const Derivative & parent_derivative);

  /// maximum derivative order
  unsigned int _derivative_order;

  /// The requested derivatives of the free energy (sorted by order)
  std::vector<Derivative> _derivatives;

  /// variable base name for the dynamically material property derivatives
  const std::string _dmatvar_base;

  /// next available variable number for automatically created material property derivative variables
  unsigned int _dmatvar_index;

  /**
   * list of all indices into _variable_names to take derivatives, w.r.t. By default this always
   * includes 0.._nargs-1, which are the coupled variables
   */
  std::vector<std::size_t> _derivative_symbol_table;

private:
  // for bulk registration of material property derivatives
  std::vector<MaterialPropertyDerivativeRule> _bulk_rules;
};

template <bool is_ad>
struct DerivativeParsedMaterialHelperTempl<is_ad>::Derivative
{
  GenericMaterialProperty<Real, is_ad> * _mat_prop;
  SymFunctionPtr _F;
  std::vector<SymbolName> _darg_names;
};

template <bool is_ad>
struct DerivativeParsedMaterialHelperTempl<is_ad>::MaterialPropertyDerivativeRule
{
  MaterialPropertyDerivativeRule(std::string p, std::string v, std::string c)
    : _parent(p), _var(v), _child(c)
  {
  }

  std::string _parent;
  std::string _var;
  std::string _child;
};

typedef DerivativeParsedMaterialHelperTempl<false> DerivativeParsedMaterialHelper;
typedef DerivativeParsedMaterialHelperTempl<true> ADDerivativeParsedMaterialHelper;
