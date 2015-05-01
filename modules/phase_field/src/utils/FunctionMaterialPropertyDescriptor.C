#include "FunctionMaterialPropertyDescriptor.h"
#include "Material.h"

FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor(const std::string & expression, Material * parent) :
    _dependent_vars(),
    _derivative_vars()
{
  size_t define = expression.find_last_of(":=");

  // expression contains a ':='
  if (define != std::string::npos)
  {
    // section before ':=' is the name used in the function expression
    _fparser_name = expression.substr(0, define);

    // parse right hand side
    parseDerivative(expression.substr(define+2));
  }
  else
  {
    // parse entire expression and use natural material property base name
    // for D(x(t),t,t) this would simply be 'x'!
    parseDerivative(expression);
    _fparser_name = _base_name;
  }

  // get the material property reference
  _value = &(parent->getMaterialProperty<Real>(getPropertyName()));
}

FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor() :
    _value(NULL)
{
}

FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor(const FunctionMaterialPropertyDescriptor & rhs) :
    _fparser_name(rhs._fparser_name),
    _base_name(rhs._base_name),
    _dependent_vars(rhs._dependent_vars),
    _derivative_vars(rhs._derivative_vars),
    _value(rhs._value)
{
}

void
FunctionMaterialPropertyDescriptor::parseDerivative(const std::string & expression)
{
  size_t open  = expression.find_first_of("[");
  size_t close = expression.find_last_of("]");

  if (open == std::string::npos && close == std::string::npos)
  {
    // no derivative requested
    parseDependentVariables(expression);
    return;
  }
  else if (open != std::string::npos && close != std::string::npos && expression.substr(0, open) == "D")
  {
    // parse argument list 0 is the function and 1,.. ar the variable to take the derivative w.r.t.
    MooseUtils::tokenize(expression.substr(open + 1, close - open - 1), _derivative_vars, 0, ",");

    // check for empty [] brackets
    if (_derivative_vars.size() > 0)
    {
      // parse argument zero of D[] as the function material property
      parseDependentVariables(_derivative_vars[0]);

      // remove function from the _derivative_vars vector
      _derivative_vars.erase(_derivative_vars.begin());
      return;
    }
  }

  mooseError("Malformed material_properties expression '" << expression << "'");
}

void
FunctionMaterialPropertyDescriptor::parseDependentVariables(const std::string & expression)
{
  size_t open  = expression.find_first_of("(");
  size_t close = expression.find_last_of(")");

  if (open == std::string::npos && close == std::string::npos)
  {
    // material property name without arguments
    _fparser_name = _base_name = expression;
  }
  else if (open != std::string::npos && close != std::string::npos)
  {
    // take material property name before bracket
    _base_name = expression.substr(0, open);

    // parse argument list
    MooseUtils::tokenize(expression.substr(open + 1, close - open - 1), _dependent_vars, 0, ",");

    // cremove duplicates from dependent variable list
    std::sort(_dependent_vars.begin(), _dependent_vars.end());
    _dependent_vars.erase(std::unique(_dependent_vars.begin(), _dependent_vars.end()), _dependent_vars.end());
  }
  else
    mooseError("Malformed material_properties expression '" << expression << "'");
}
