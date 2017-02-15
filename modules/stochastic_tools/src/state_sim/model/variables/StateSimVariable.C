/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimVariable.h"
#include "StateSimModel.h"
#include <string>
#include <stdlib.h>

StateSimVariable::StateSimVariable(StateSimModel & main_model, const std::string &  name, VAR_SCOPE_ENUM scope, const Real & value)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::VARIABLE), name),
    _value(""),
    _value_ptr(NULL),
    _var_type(VAR_TYPE::VT_DOUBLE),
    _var_scope(scope)
{
  _value = std::to_string(value);
}

StateSimVariable::StateSimVariable(StateSimModel & main_model, const std::string &  name, VAR_SCOPE_ENUM scope, const std::string & value)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::VARIABLE), name),
    _value(value),
    _value_ptr(NULL),
    _var_type(VAR_TYPE::VT_STRING),
    _var_scope(scope)
{
}

StateSimVariable::StateSimVariable(StateSimModel & main_model, const std::string &  name, VAR_SCOPE_ENUM scope, const bool & value)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::VARIABLE), name),
    _value(""),
    _value_ptr(NULL),
    _var_type(VAR_TYPE::VT_BOOL),
    _var_scope(scope)
{
  _value = value ? "true" : "false";
}

StateSimVariable::StateSimVariable(StateSimModel & main_model, const std::string &  name, Real & value)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::VARIABLE), name),
    _value(""),
    _value_ptr(&value),
    _var_type(VAR_TYPE::VT_DOUBLE_PTR),
    _var_scope(VAR_SCOPE_ENUM::VS_MOOSE)
{
}

std::string
StateSimVariable::getTypeName() const
{
  switch (_var_type)
  {
    case VAR_TYPE::VT_DOUBLE:
      return "Real";
    case VAR_TYPE::VT_STRING:
      return "string";
    case VAR_TYPE::VT_BOOL:
      return "bool";
    case VAR_TYPE::VT_DOUBLE_PTR:
      return "Real ptr";
    default :
      return "var type name missing";
  }
}

void
StateSimVariable::setReal(const Real & value)
{
  _value = std::to_string(value);
}

Real
StateSimVariable::getReal()
{
  mooseAssert((_var_type == VAR_TYPE::VT_DOUBLE), "Variable named '" + this->name() + "' is a " + this->getTypeName() + " but getReal called.");

  Real ret = atof(_value.c_str());
  return ret;
}

std::string
StateSimVariable::getString()
{
  mooseAssert((_var_type == VAR_TYPE::VT_STRING), "Variable named '" + this->name() + "' is a " + this->getTypeName() + " but getString called.");
  return _value;
}

bool
StateSimVariable::getBool()
{
  mooseAssert((_var_type == VAR_TYPE::VT_BOOL), "Variable named '" + this->name() + "' is a " + this->getTypeName() + " but getBool called.");
  return _value == "true";
}

Real
StateSimVariable::getRealPtrVal()
{
  mooseAssert((_var_type == VAR_TYPE::VT_DOUBLE_PTR), "Variable named '" + this->name() + "' is a " + this->getTypeName() + " but getRealPtrVal called.");
  return *_value_ptr;
}

Real
StateSimVariable::getReal() const
{
  mooseAssert((_var_type == VAR_TYPE::VT_DOUBLE), "Variable named '" + this->name() + "' is a " + this->getTypeName() + " but getReal called.");

  Real ret = atof(_value.c_str());
  return ret;
}

std::string
StateSimVariable::getString() const
{
  mooseAssert((_var_type == VAR_TYPE::VT_STRING), "Variable named '" + this->name() + "' is a " + this->getTypeName() + " but getString called.");
  return _value;
}

bool
StateSimVariable::getBool() const
{
  mooseAssert((_var_type == VAR_TYPE::VT_BOOL), "Variable named '" + this->name() + "' is a " + this->getTypeName() + " but getBool called.");
  return _value == "true";
}

Real
StateSimVariable::getRealPtrVal() const
{
  mooseAssert((_var_type == VAR_TYPE::VT_DOUBLE_PTR), "Variable named '" + this->name() + "' is a " + this->getTypeName() + " but getRealPtrVal called.");
  return *_value_ptr;
}
