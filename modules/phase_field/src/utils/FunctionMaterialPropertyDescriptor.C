#include "FunctionMaterialPropertyDescriptor.h"
#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "Kernel.h"
#include <algorithm>

FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor(
    const std::string & expression, MooseObject * parent)
  : _dependent_vars(), _derivative_vars(), _parent(parent)
{
  size_t define = expression.find_last_of(":=");

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

  _value = NULL;
}

FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor() : _value(NULL) {}

FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor(
    const FunctionMaterialPropertyDescriptor & rhs)
  : _fparser_name(rhs._fparser_name),
    _base_name(rhs._base_name),
    _dependent_vars(rhs._dependent_vars),
    _derivative_vars(rhs._derivative_vars),
    _value(NULL),
    _parent(rhs._parent)
{
}

std::vector<FunctionMaterialPropertyDescriptor>
FunctionMaterialPropertyDescriptor::parseVector(const std::vector<std::string> & expression_list,
                                                MooseObject * parent)
{
  std::vector<FunctionMaterialPropertyDescriptor> fmpds;
  for (std::vector<std::string>::const_iterator i = expression_list.begin();
       i != expression_list.end();
       ++i)
    fmpds.push_back(FunctionMaterialPropertyDescriptor(*i, parent));
  return fmpds;
}

void
FunctionMaterialPropertyDescriptor::addDerivative(const VariableName & var)
{
  _derivative_vars.push_back(var);
  _value = NULL;
}

bool
FunctionMaterialPropertyDescriptor::dependsOn(const std::string & var) const
{
  return std::find(_dependent_vars.begin(), _dependent_vars.end(), var) != _dependent_vars.end() ||
         std::find(_derivative_vars.begin(), _derivative_vars.end(), var) != _derivative_vars.end();
}

std::vector<VariableName>
FunctionMaterialPropertyDescriptor::getDependentVariables()
{
  std::set<VariableName> all;
  all.insert(_dependent_vars.begin(), _dependent_vars.end());
  all.insert(_derivative_vars.begin(), _derivative_vars.end());

  return std::vector<VariableName>(all.begin(), all.end());
}

void
FunctionMaterialPropertyDescriptor::parseDerivative(const std::string & expression)
{
  size_t open = expression.find_first_of("[");
  size_t close = expression.find_last_of("]");

  if (open == std::string::npos && close == std::string::npos)
  {
    // no derivative requested
    parseDependentVariables(expression);

    return;
  }
  else if (open != std::string::npos && close != std::string::npos &&
           expression.substr(0, open) == "D")
  {
    // tokenize splits the arguments in d2h2:=D[h2(eta1,eta2),eta1] into 'h2(eta1' 'eta2)' 'eta1'
    // DAMN!!
    std::string arguments = expression.substr(open + 1, close - open - 1);
    size_t close2 = arguments.find_last_of(")");

    if (close2 == std::string::npos)
    {
      // rest of argument list 0 is the function and 1,.. are the variable to take the derivative
      // w.r.t.
      MooseUtils::tokenize(arguments, _derivative_vars, 0, ",");

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
    else
    {
      parseDependentVariables(arguments.substr(0, close2 + 1));
      MooseUtils::tokenize(arguments.substr(close2 + 2), _derivative_vars, 0, ",");
      return;
    }
  }

  mooseError("Malformed material_properties expression '", expression, "'");
}

void
FunctionMaterialPropertyDescriptor::parseDependentVariables(const std::string & expression)
{
  size_t open = expression.find_first_of("(");
  size_t close = expression.find_last_of(")");

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
    MooseUtils::tokenize(expression.substr(open + 1, close - open - 1), _dependent_vars, 0, ",");

    // cremove duplicates from dependent variable list
    std::sort(_dependent_vars.begin(), _dependent_vars.end());
    _dependent_vars.erase(std::unique(_dependent_vars.begin(), _dependent_vars.end()),
                          _dependent_vars.end());
  }
  else
    mooseError("Malformed material_properties expression '", expression, "'");
}

void
FunctionMaterialPropertyDescriptor::printDebug()
{
  Moose::out << "MPD: " << _fparser_name << ' ' << _base_name << " deriv = [";
  for (unsigned int i = 0; i < _derivative_vars.size(); ++i)
    Moose::out << _derivative_vars[i] << ' ';
  Moose::out << "] dep = [";
  for (unsigned int i = 0; i < _dependent_vars.size(); ++i)
    Moose::out << _dependent_vars[i] << ' ';
  Moose::out << "] " << getPropertyName() << '\n';
}

const MaterialProperty<Real> &
FunctionMaterialPropertyDescriptor::value() const
{
  if (_value == NULL)
  {
    DerivativeMaterialInterface<Material> * _material_parent =
        dynamic_cast<DerivativeMaterialInterface<Material> *>(_parent);
    DerivativeMaterialInterface<Kernel> * _kernel_parent =
        dynamic_cast<DerivativeMaterialInterface<Kernel> *>(_parent);

    // get the material property reference
    if (_material_parent)
      _value =
          &(_material_parent->getMaterialPropertyDerivative<Real>(_base_name, _derivative_vars));
    else if (_kernel_parent)
      _value = &(_kernel_parent->getMaterialPropertyDerivative<Real>(_base_name, _derivative_vars));
    else
      mooseError("A FunctionMaterialPropertyDescriptor must be owned by either a Material or a "
                 "Kernel object.");
  }

  return *_value;
}
