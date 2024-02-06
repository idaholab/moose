//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionMaterialPropertyDescriptor.h"
#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "Kernel.h"
#include <algorithm>

template <bool is_ad>
FunctionMaterialPropertyDescriptor<is_ad>::FunctionMaterialPropertyDescriptor(
    const std::string & expression, MooseObject * parent, bool required)
  : _state(PropertyState::CURRENT),
    _dependent_symbols(),
    _derivative_symbols(),
    _value(nullptr),
    _old_older_value(nullptr),
    _parent(parent),
    _required(required)
{
  auto define = expression.find_last_of(":=");

  // expression contains a ':='
  if (define != std::string::npos)
  {
    // section before ':=' is the name used in the function expression
    _fparser_name = expression.substr(0, define - 1);

    // parse right hand side
    parseDerivative(expression.substr(define + 1));
  }
  else
  {
    // parse entire expression and use natural material property base name
    // for D(x(t),t,t) this would simply be 'x'!
    parseDerivative(expression);
    _fparser_name = _base_name;
  }

  updatePropertyName();
}

template <bool is_ad>
FunctionMaterialPropertyDescriptor<is_ad>::FunctionMaterialPropertyDescriptor(
    const FunctionMaterialPropertyDescriptor & rhs)
  : _state(rhs._state),
    _fparser_name(rhs._fparser_name),
    _base_name(rhs._base_name),
    _dependent_symbols(rhs._dependent_symbols),
    _derivative_symbols(rhs._derivative_symbols),
    _value(nullptr),
    _old_older_value(nullptr),
    _parent(rhs._parent),
    _property_name(rhs._property_name),
    _required(false)
{
}

template <bool is_ad>
FunctionMaterialPropertyDescriptor<is_ad>::FunctionMaterialPropertyDescriptor(
    const FunctionMaterialPropertyDescriptor & rhs, MooseObject * parent)
  : _state(rhs._state),
    _fparser_name(rhs._fparser_name),
    _base_name(rhs._base_name),
    _dependent_symbols(rhs._dependent_symbols),
    _derivative_symbols(rhs._derivative_symbols),
    _value(nullptr),
    _old_older_value(nullptr),
    _parent(parent),
    _property_name(rhs._property_name),
    _required(false)
{
}

template <bool is_ad>
std::vector<FunctionMaterialPropertyDescriptor<is_ad>>
FunctionMaterialPropertyDescriptor<is_ad>::parseVector(
    const std::vector<std::string> & expression_list, MooseObject * parent)
{
  std::vector<FunctionMaterialPropertyDescriptor> fmpds;
  for (auto & ex : expression_list)
    fmpds.push_back(FunctionMaterialPropertyDescriptor(ex, parent));
  return fmpds;
}

template <bool is_ad>
void
FunctionMaterialPropertyDescriptor<is_ad>::addDerivative(const SymbolName & var)
{
  _derivative_symbols.push_back(var);
  _value = nullptr;
  updatePropertyName();
}

template <bool is_ad>
bool
FunctionMaterialPropertyDescriptor<is_ad>::dependsOn(const std::string & var) const
{
  return std::find(_dependent_symbols.begin(), _dependent_symbols.end(), var) !=
             _dependent_symbols.end() ||
         std::find(_derivative_symbols.begin(), _derivative_symbols.end(), var) !=
             _derivative_symbols.end();
}

template <bool is_ad>
std::vector<DerivativeMaterialPropertyNameInterface::SymbolName>
FunctionMaterialPropertyDescriptor<is_ad>::getDependentSymbols()
{
  std::set<SymbolName> all(_dependent_symbols.begin(), _dependent_symbols.end());
  all.insert(_derivative_symbols.begin(), _derivative_symbols.end());

  return std::vector<SymbolName>(all.begin(), all.end());
}

template <bool is_ad>
void
FunctionMaterialPropertyDescriptor<is_ad>::parseDerivative(const std::string & expression)
{
  auto open = expression.find_first_of("[");
  auto close = expression.find_last_of("]");

  if (open == std::string::npos && close == std::string::npos)
  {
    // no derivative requested
    parseDependentSymbols(expression);

    return;
  }
  if (open != std::string::npos && close != std::string::npos)
  {
    if (expression.substr(0, open) == "Old")
    {
      _base_name = expression.substr(open + 1, close - open - 1);
      _dependent_symbols.clear();
      _state = PropertyState::OLD;
      return;
    }
    if (expression.substr(0, open) == "Older")
    {
      _base_name = expression.substr(open + 1, close - open - 1);
      _dependent_symbols.clear();
      _state = PropertyState::OLDER;
      return;
    }
    else if (expression.substr(0, open) == "D")
    {
      auto arguments = expression.substr(open + 1, close - open - 1);
      auto close2 = arguments.find_last_of(")");

      if (close2 == std::string::npos)
      {
        // rest of argument list 0 is the function and 1,.. are the variable to take the derivative
        // w.r.t.
        MooseUtils::tokenize(arguments, _derivative_symbols, 0, ",");

        // check for empty [] brackets
        if (_derivative_symbols.size() > 0)
        {
          // parse argument zero of D[] as the function material property
          parseDependentSymbols(_derivative_symbols[0]);

          // remove function from the _derivative_symbols vector
          _derivative_symbols.erase(_derivative_symbols.begin());
          updatePropertyName();

          return;
        }
      }
      else
      {
        parseDependentSymbols(arguments.substr(0, close2 + 1));
        MooseUtils::tokenize(arguments.substr(close2 + 2), _derivative_symbols, 0, ",");
        updatePropertyName();
        return;
      }
    }
  }

  mooseError("Malformed material_properties expression '", expression, "'");
}

template <bool is_ad>
void
FunctionMaterialPropertyDescriptor<is_ad>::parseDependentSymbols(const std::string & expression)
{
  auto open = expression.find_first_of("(");
  auto close = expression.find_last_of(")");

  if (open == std::string::npos && close == std::string::npos)
  {
    // material property name without arguments
    _base_name = expression;
  }
  else if (open != std::string::npos && close != std::string::npos)
  {
    // take material property name before bracket
    _base_name = expression.substr(0, open);

    // parse argument list
    MooseUtils::tokenize(expression.substr(open + 1, close - open - 1), _dependent_symbols, 0, ",");

    // remove duplicates from dependent variable list
    std::sort(_dependent_symbols.begin(), _dependent_symbols.end());
    _dependent_symbols.erase(std::unique(_dependent_symbols.begin(), _dependent_symbols.end()),
                             _dependent_symbols.end());
  }
  else
    mooseError("Malformed material_properties expression '", expression, "'");
}

template <bool is_ad>
void
FunctionMaterialPropertyDescriptor<is_ad>::printDebug()
{
  Moose::out << "MPD: " << _fparser_name << ' ' << _base_name << " deriv = [";
  for (auto & dv : _derivative_symbols)
    Moose::out << dv << ' ';
  Moose::out << "] dep = [";
  for (auto & dv : _dependent_symbols)
    Moose::out << dv << ' ';
  Moose::out << "] " << getPropertyName() << '\n';
}

template <bool is_ad>
GenericReal<is_ad>
FunctionMaterialPropertyDescriptor<is_ad>::value(unsigned int qp) const
{
  // current property
  if (_state == PropertyState::CURRENT)
  {
    if (_value == nullptr)
    {
      DerivativeMaterialInterface<Material> * _material_parent =
          dynamic_cast<DerivativeMaterialInterface<Material> *>(_parent);
      DerivativeMaterialInterface<Kernel> * _kernel_parent =
          dynamic_cast<DerivativeMaterialInterface<Kernel> *>(_parent);

      // property name
      auto name = derivativePropertyName(_base_name, _derivative_symbols);

      // get the material property reference
      if (_material_parent)
        _value = _required ? &(_material_parent->getGenericMaterialProperty<Real, is_ad>(name))
                           : &(_material_parent->getGenericZeroMaterialProperty<Real, is_ad>(name));
      else if (_kernel_parent)
        _value = _required ? &(_kernel_parent->getGenericMaterialProperty<Real, is_ad>(name))
                           : &(_kernel_parent->getGenericZeroMaterialProperty<Real, is_ad>(name));
      else
        mooseError("A FunctionMaterialPropertyDescriptor must be owned by either a Material or a "
                   "Kernel object.");
    }

    return qp != libMesh::invalid_uint ? (*_value)[qp] : 0.0;
  }

  // old or older property
  if (_old_older_value == nullptr)
  {
    mooseAssert(_derivative_symbols.empty(), "Don't take derivatives of old/older properties.");

    DerivativeMaterialInterface<Material> * _material_parent =
        dynamic_cast<DerivativeMaterialInterface<Material> *>(_parent);
    DerivativeMaterialInterface<Kernel> * _kernel_parent =
        dynamic_cast<DerivativeMaterialInterface<Kernel> *>(_parent);

    // get the material property reference
    if (_material_parent)
      _old_older_value = (_state == PropertyState::OLD)
                             ? &(_material_parent->getMaterialPropertyOld<Real>(_base_name))
                             : &(_material_parent->getMaterialPropertyOlder<Real>(_base_name));
    else if (_kernel_parent)
      _old_older_value = (_state == PropertyState::OLD)
                             ? &(_kernel_parent->getMaterialPropertyOld<Real>(_base_name))
                             : &(_kernel_parent->getMaterialPropertyOlder<Real>(_base_name));
    else
      mooseError("A FunctionMaterialPropertyDescriptor must be owned by either a Material or a "
                 "Kernel object.");
  }

  return qp != libMesh::invalid_uint ? (*_old_older_value)[qp] : 0.0;
}

template <bool is_ad>
void
FunctionMaterialPropertyDescriptor<is_ad>::updatePropertyName()
{
  _property_name = derivativePropertyName(_base_name, _derivative_symbols);
}

// explicit instantiation
template class FunctionMaterialPropertyDescriptor<false>;
template class FunctionMaterialPropertyDescriptor<true>;
