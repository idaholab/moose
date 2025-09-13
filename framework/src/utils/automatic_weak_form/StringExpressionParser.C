#include "StringExpressionParser.h"
#include "MooseError.h"

namespace moose
{
namespace automatic_weak_form
{

void StringExpressionParser::setVectorParameter(const std::string & name, const RealVectorValue & value)
{
  _vector_parameters[name] = value;
}

void StringExpressionParser::setTensorParameter(const std::string & name, const RankTwoTensor & value)
{
  _tensor_parameters[name] = value;
}

NodePtr StringExpressionParser::expandCustomFunction(const std::string & name, const std::vector<NodePtr> & args)
{
  if (!_custom_functions.count(name))
    mooseError("Unknown custom function: " + name);
  
  // For now, just create a function node
  // TODO: Implement actual expansion with parameter substitution
  return function(name, args);
}

NodePtr StringExpressionParser::parseCurl(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("curl() requires exactly 1 argument");
  
  if (_dim != 3)
    mooseError("curl() only supported in 3D");
  
  // curl = ∇ × v
  return curl(args[0]);
}

NodePtr StringExpressionParser::parseCross(const std::vector<NodePtr> & args)
{
  if (args.size() != 2)
    mooseError("cross() requires exactly 2 arguments");
  
  if (_dim != 3)
    mooseError("cross() only supported in 3D");
  
  return cross(args[0], args[1]);
}

NodePtr StringExpressionParser::parseNormalize(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("normalize() requires exactly 1 argument");
  
  return normalize(args[0]);
}

NodePtr StringExpressionParser::parseSkew(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("skew() requires exactly 1 argument");
  
  return skew(args[0]);
}

NodePtr StringExpressionParser::parseDev(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("dev() requires exactly 1 argument");
  
  return dev(args[0]);
}

NodePtr StringExpressionParser::parseContract(const std::vector<NodePtr> & args)
{
  if (args.size() != 2)
    mooseError("contract() requires exactly 2 arguments");
  
  return contract(args[0], args[1]);
}

NodePtr StringExpressionParser::parseOuter(const std::vector<NodePtr> & args)
{
  if (args.size() != 2)
    mooseError("outer() requires exactly 2 arguments");
  
  return outer(args[0], args[1]);
}

NodePtr StringExpressionParser::parseTan(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("tan() requires exactly 1 argument");
  
  return function("tan", args);
}

NodePtr StringExpressionParser::parseAbs(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("abs() requires exactly 1 argument");
  
  return function("abs", args);
}

NodePtr StringExpressionParser::parseGinzburgLandau(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("GinzburgLandau() requires exactly 1 argument");
  
  // Standard Ginzburg-Landau potential: (|ψ|^2 - 1)^2
  auto psi = args[0];
  auto psi2 = moose::automatic_weak_form::dot(psi, psi);
  auto psi2_minus_1 = subtract(psi2, constant(1.0));
  return multiply(psi2_minus_1, psi2_minus_1);
}

}
}