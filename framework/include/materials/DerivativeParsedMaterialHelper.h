//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"
#include "ParsedMaterialHelper.h"
#include "libmesh/fparser_ad.hh"

// Forward Declarations
template <typename T = Real>
class DerivativeParsedMaterialHelper;

template <>
InputParameters validParams<DerivativeParsedMaterialHelper<> >();

/**
 * Helper class to perform the auto derivative taking.
 */
template <typename T>
class DerivativeParsedMaterialHelper : public ParsedMaterialHelper<T>
{
public:
  DerivativeParsedMaterialHelper(const InputParameters & parameters,
                                 typename ParsedMaterialHelper<T>::VariableNameMappingMode map_mode = ParsedMaterialHelper<T>::USE_PARAM_NAMES);

protected:
  struct Derivative;
  struct MaterialPropertyDerivativeRule;

  virtual void initQpStatefulProperties();
  virtual void functionValue();

  virtual void functionsPostParse();
  void assembleDerivatives();

  void
  recurseMatProps(unsigned int var, unsigned int order, const typename ParsedMaterialHelper<T>::MatPropDescriptorList & parent_mpd);

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

private:
  // for bulk registration of material property derivatives
  std::vector<MaterialPropertyDerivativeRule> _bulk_rules;
};

template <typename T>
struct DerivativeParsedMaterialHelper<T>::Derivative
{
  MaterialProperty<T> * _mat_prop;
  std::vector<typename ParsedMaterialHelper<T>::ADFunctionPtr> _F;
  std::vector<VariableName> _darg_names;
};

template <typename T>
struct DerivativeParsedMaterialHelper<T>::MaterialPropertyDerivativeRule
{
  MaterialPropertyDerivativeRule(std::string p, std::string v, std::string c)
    : _parent(p), _var(v), _child(c)
  {
  }

  std::string _parent;
  std::string _var;
  std::string _child;
};
