#pragma once

#include "MooseAST.h"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <sstream>
#include <cctype>
#include <stdexcept>

namespace moose
{
namespace automatic_weak_form
{

class StringExpressionParser
{
public:
  StringExpressionParser(unsigned int dim = 3) : _dim(dim) {}
  
  // Parse a complete energy functional expression
  // Example: "W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*norm(hessian(c))^2"
  NodePtr parse(const std::string & expression);
  
  // Parse multiple coupled equations
  // Example: {"mu = dW/dc - kappa*laplacian(c)", "dc/dt = div(M*grad(mu))"}
  std::map<std::string, NodePtr> parseSystem(const std::vector<std::string> & equations);
  
  // Register custom functions with their derivatives
  void registerFunction(const std::string & name, 
                        const std::string & expression,
                        const std::map<std::string, std::string> & derivatives = {});
  
  // Set parameter values
  void setParameter(const std::string & name, Real value);
  void setVectorParameter(const std::string & name, const RealVectorValue & value);
  void setTensorParameter(const std::string & name, const RankTwoTensor & value);
  
  // Define variables
  void defineVariable(const std::string & name, const Shape & shape = ScalarShape{});
  
  // Define intermediate expressions (e.g., strain = sym(grad(u)))
  void defineExpression(const std::string & name, const std::string & expression);
  void defineExpression(const std::string & name, const NodePtr & expr);
  
  // Parse semicolon-separated expressions  
  void parseExpressions(const std::string & expr_list);
  
  // Parse multiple energy functionals for coupled systems
  std::map<std::string, NodePtr> parseMultipleEnergies(
      const std::map<std::string, std::string> & energy_expressions);
  
  // Strong form equation types
  enum class EquationType
  {
    STEADY,      // 0 = F (no time derivative)
    TRANSIENT    // du/dt = F (has time derivative)
  };
  
  struct StrongFormEquation
  {
    std::string variable;
    EquationType type;
    NodePtr rhs;  // Right-hand side expression
    NodePtr weak_residual;  // Derived weak form residual
    NodePtr jacobian;  // Derived Jacobian
  };
  
  // Parse strong form equations (e.g., "c_t = -div(M*grad(mu)); mu = dW/dc - kappa*laplacian(c)")
  std::map<std::string, StrongFormEquation> parseStrongForms(const std::string & equations);
  
private:
  unsigned int _dim;
  std::map<std::string, Real> _parameters;
  std::map<std::string, RealVectorValue> _vector_parameters;
  std::map<std::string, RankTwoTensor> _tensor_parameters;
  std::map<std::string, Shape> _variables;
  std::map<std::string, std::string> _custom_functions;
  std::map<std::string, std::map<std::string, std::string>> _custom_derivatives;
  std::map<std::string, NodePtr> _expressions;  // Stored intermediate expressions
  std::map<std::string, std::string> _expression_strings;  // String versions for parsing
  
  // Tokenizer
  struct Token
  {
    enum Type
    {
      NUMBER,
      IDENTIFIER,
      OPERATOR,
      LPAREN,
      RPAREN,
      LBRACKET,
      RBRACKET,
      COMMA,
      EQUALS,
      END
    };
    
    Type type;
    std::string value;
    Real number;
    
    Token(Type t = END, const std::string & v = "", Real n = 0) 
      : type(t), value(v), number(n) {}
  };
  
  class Tokenizer
  {
  public:
    Tokenizer(const std::string & expr) : _expr(expr), _pos(0) {}
    
    Token nextToken();
    Token peekToken();
    void rewind() { if (_pos > 0) _pos--; }
    
  private:
    std::string _expr;
    size_t _pos;
    Token _last_token;
    
    void skipWhitespace();
    Token readNumber();
    Token readIdentifier();
    Token readOperator();
  };
  
  // Recursive descent parser
  NodePtr parseExpression(Tokenizer & tokenizer);
  NodePtr parseAdditive(Tokenizer & tokenizer);
  NodePtr parseMultiplicative(Tokenizer & tokenizer);
  NodePtr parsePower(Tokenizer & tokenizer);
  NodePtr parseUnary(Tokenizer & tokenizer);
  NodePtr parsePrimary(Tokenizer & tokenizer);
  NodePtr parseFunction(const std::string & name, Tokenizer & tokenizer);
  NodePtr parseDerivative(Tokenizer & tokenizer);
  
  // Helper functions
  NodePtr createParameterNode(const std::string & name);
  NodePtr createVariableNode(const std::string & name);
  NodePtr expandCustomFunction(const std::string & name, const std::vector<NodePtr> & args);
  
  // Built-in function handlers
  NodePtr parseGradient(const std::vector<NodePtr> & args);
  NodePtr parseDivergence(const std::vector<NodePtr> & args);
  NodePtr parseLaplacian(const std::vector<NodePtr> & args);
  NodePtr parseHessian(const std::vector<NodePtr> & args);
  NodePtr parseCurl(const std::vector<NodePtr> & args);
  NodePtr parseDot(const std::vector<NodePtr> & args);
  NodePtr parseCross(const std::vector<NodePtr> & args);
  NodePtr parseNorm(const std::vector<NodePtr> & args);
  NodePtr parseNormalize(const std::vector<NodePtr> & args);
  NodePtr parseTrace(const std::vector<NodePtr> & args);
  NodePtr parseDet(const std::vector<NodePtr> & args);
  NodePtr parseInv(const std::vector<NodePtr> & args);
  NodePtr parseTranspose(const std::vector<NodePtr> & args);
  NodePtr parseSym(const std::vector<NodePtr> & args);
  NodePtr parseSkew(const std::vector<NodePtr> & args);
  NodePtr parseDev(const std::vector<NodePtr> & args);
  NodePtr parseContract(const std::vector<NodePtr> & args);
  NodePtr parseOuter(const std::vector<NodePtr> & args);
  NodePtr parseVec(const std::vector<NodePtr> & args);  // Assemble vector from components
  
  // Standard mathematical functions
  NodePtr parseExp(const std::vector<NodePtr> & args);
  NodePtr parseLog(const std::vector<NodePtr> & args);
  NodePtr parseSin(const std::vector<NodePtr> & args);
  NodePtr parseCos(const std::vector<NodePtr> & args);
  NodePtr parseTan(const std::vector<NodePtr> & args);
  NodePtr parseSqrt(const std::vector<NodePtr> & args);
  NodePtr parseAbs(const std::vector<NodePtr> & args);
  NodePtr parsePow(const std::vector<NodePtr> & args);
  
  // Common energy functions
  NodePtr parseDoubleWell(const std::vector<NodePtr> & args);
  NodePtr parseGinzburgLandau(const std::vector<NodePtr> & args);
};

// Convenience function for parsing
inline NodePtr parseExpression(const std::string & expr, unsigned int dim = 3)
{
  StringExpressionParser parser(dim);
  return parser.parse(expr);
}

// Implementation of StringExpressionParser methods

inline NodePtr StringExpressionParser::parse(const std::string & expression)
{
  Tokenizer tokenizer(expression);
  return parseExpression(tokenizer);
}

inline std::map<std::string, NodePtr> 
StringExpressionParser::parseSystem(const std::vector<std::string> & equations)
{
  std::map<std::string, NodePtr> system;
  
  for (const auto & eq : equations)
  {
    size_t eq_pos = eq.find('=');
    if (eq_pos == std::string::npos)
      mooseError("Invalid equation format: " + eq);
    
    std::string lhs = eq.substr(0, eq_pos);
    std::string rhs = eq.substr(eq_pos + 1);
    
    // Trim whitespace
    lhs.erase(0, lhs.find_first_not_of(" \t"));
    lhs.erase(lhs.find_last_not_of(" \t") + 1);
    
    system[lhs] = parse(rhs);
  }
  
  return system;
}

inline void StringExpressionParser::registerFunction(
    const std::string & name,
    const std::string & expression,
    const std::map<std::string, std::string> & derivatives)
{
  _custom_functions[name] = expression;
  _custom_derivatives[name] = derivatives;
}

inline void StringExpressionParser::setParameter(const std::string & name, Real value)
{
  _parameters[name] = value;
}

inline void StringExpressionParser::defineVariable(const std::string & name, const Shape & shape)
{
  _variables[name] = shape;
}

inline void StringExpressionParser::defineExpression(const std::string & name, const std::string & expression)
{
  _expression_strings[name] = expression;
  _expressions[name] = parse(expression);
}

inline void StringExpressionParser::defineExpression(const std::string & name, const NodePtr & expr)
{
  _expressions[name] = expr;
}

inline void StringExpressionParser::parseExpressions(const std::string & expr_list)
{
  // Split by semicolons
  std::vector<std::string> expressions;
  std::stringstream ss(expr_list);
  std::string expr;
  
  while (std::getline(ss, expr, ';'))
  {
    // Trim whitespace
    expr.erase(0, expr.find_first_not_of(" \t\n"));
    expr.erase(expr.find_last_not_of(" \t\n") + 1);
    
    if (expr.empty())
      continue;
    
    // Look for assignment
    size_t eq_pos = expr.find('=');
    if (eq_pos != std::string::npos)
    {
      std::string name = expr.substr(0, eq_pos);
      std::string rhs = expr.substr(eq_pos + 1);
      
      // Trim whitespace from name and rhs
      name.erase(0, name.find_first_not_of(" \t"));
      name.erase(name.find_last_not_of(" \t") + 1);
      rhs.erase(0, rhs.find_first_not_of(" \t"));
      rhs.erase(rhs.find_last_not_of(" \t") + 1);
      
      defineExpression(name, rhs);
    }
  }
}

inline std::map<std::string, StringExpressionParser::StrongFormEquation>
StringExpressionParser::parseStrongForms(const std::string & equations)
{
  std::map<std::string, StrongFormEquation> result;
  
  // Split by semicolons
  std::vector<std::string> eqns;
  std::stringstream ss(equations);
  std::string eq;
  
  while (std::getline(ss, eq, ';'))
  {
    // Trim whitespace
    eq.erase(0, eq.find_first_not_of(" \t\n"));
    eq.erase(eq.find_last_not_of(" \t\n") + 1);
    
    if (eq.empty())
      continue;
    
    // Look for equals sign
    size_t eq_pos = eq.find('=');
    if (eq_pos == std::string::npos)
      mooseError("Invalid equation format (missing '='): " + eq);
    
    std::string lhs = eq.substr(0, eq_pos);
    std::string rhs = eq.substr(eq_pos + 1);
    
    // Trim whitespace
    lhs.erase(0, lhs.find_first_not_of(" \t"));
    lhs.erase(lhs.find_last_not_of(" \t") + 1);
    rhs.erase(0, rhs.find_first_not_of(" \t"));
    rhs.erase(rhs.find_last_not_of(" \t") + 1);
    
    StrongFormEquation sfe;
    
    // Check if LHS has time derivative notation
    if (lhs.size() > 2 && lhs.substr(lhs.size() - 2) == "_t")
    {
      // Time derivative: c_t = F
      sfe.variable = lhs.substr(0, lhs.size() - 2);
      sfe.type = EquationType::TRANSIENT;
    }
    else if (lhs.size() > 4 && lhs.substr(0, 3) == "dt(" && lhs.back() == ')')
    {
      // Alternative notation: dt(c) = F
      sfe.variable = lhs.substr(3, lhs.size() - 4);
      sfe.type = EquationType::TRANSIENT;
    }
    else
    {
      // Steady state: c = F means 0 = F - c, but usually we want mu = expression
      sfe.variable = lhs;
      sfe.type = EquationType::STEADY;
    }
    
    // Parse the RHS expression
    sfe.rhs = parse(rhs);
    
    // TODO: Derive weak form and Jacobian here
    // For now, just store the RHS
    sfe.weak_residual = sfe.rhs;
    sfe.jacobian = nullptr;
    
    result[sfe.variable] = sfe;
  }
  
  return result;
}

inline std::map<std::string, NodePtr> 
StringExpressionParser::parseMultipleEnergies(const std::map<std::string, std::string> & energy_expressions)
{
  std::map<std::string, NodePtr> energies;
  for (const auto & [var_name, expr] : energy_expressions)
  {
    energies[var_name] = parse(expr);
  }
  return energies;
}

inline NodePtr StringExpressionParser::parseExpression(Tokenizer & tokenizer)
{
  return parseAdditive(tokenizer);
}

inline NodePtr StringExpressionParser::parseAdditive(Tokenizer & tokenizer)
{
  NodePtr left = parseMultiplicative(tokenizer);
  
  while (true)
  {
    Token op = tokenizer.peekToken();
    if (op.type != Token::OPERATOR || (op.value != "+" && op.value != "-"))
      break;
    
    tokenizer.nextToken(); // consume operator
    NodePtr right = parseMultiplicative(tokenizer);
    
    if (op.value == "+")
      left = add(left, right);
    else
      left = subtract(left, right);
  }
  
  return left;
}

inline NodePtr StringExpressionParser::parseMultiplicative(Tokenizer & tokenizer)
{
  NodePtr left = parsePower(tokenizer);
  
  while (true)
  {
    Token op = tokenizer.peekToken();
    if (op.type != Token::OPERATOR || (op.value != "*" && op.value != "/"))
      break;
    
    tokenizer.nextToken(); // consume operator
    NodePtr right = parsePower(tokenizer);
    
    if (op.value == "*")
      left = multiply(left, right);
    else
      left = divide(left, right);
  }
  
  return left;
}

inline NodePtr StringExpressionParser::parsePower(Tokenizer & tokenizer)
{
  NodePtr left = parseUnary(tokenizer);
  
  Token op = tokenizer.peekToken();
  if (op.type == Token::OPERATOR && op.value == "^")
  {
    tokenizer.nextToken(); // consume ^
    NodePtr right = parsePower(tokenizer); // right associative
    left = power(left, right);
  }
  
  return left;
}

inline NodePtr StringExpressionParser::parseUnary(Tokenizer & tokenizer)
{
  Token token = tokenizer.peekToken();
  
  if (token.type == Token::OPERATOR && token.value == "-")
  {
    tokenizer.nextToken(); // consume -
    return negate(parseUnary(tokenizer));
  }
  else if (token.type == Token::OPERATOR && token.value == "+")
  {
    tokenizer.nextToken(); // consume +
    return parseUnary(tokenizer);
  }
  
  return parsePrimary(tokenizer);
}

inline NodePtr StringExpressionParser::parsePrimary(Tokenizer & tokenizer)
{
  Token token = tokenizer.nextToken();
  
  switch (token.type)
  {
    case Token::NUMBER:
      return constant(token.number);
    
    case Token::IDENTIFIER:
    {
      Token next = tokenizer.peekToken();
      if (next.type == Token::LPAREN)
      {
        return parseFunction(token.value, tokenizer);
      }
      else
      {
        // Check if it's a defined expression, parameter, or variable
        if (_expressions.count(token.value))
          return _expressions[token.value];
        else if (_parameters.count(token.value))
          return constant(_parameters[token.value]);
        else if (_variables.count(token.value))
          return fieldVariable(token.value, _variables[token.value]);
        else
          return variable(token.value);
      }
    }
    
    case Token::LPAREN:
    {
      NodePtr expr = parseExpression(tokenizer);
      Token rparen = tokenizer.nextToken();
      if (rparen.type != Token::RPAREN)
        mooseError("Expected closing parenthesis");
      return expr;
    }
    
    default:
      if (token.type == Token::END)
        mooseError("Unexpected end of expression");
      else
        mooseError("Unexpected token: '" + token.value + "'");
  }
}

inline NodePtr StringExpressionParser::parseFunction(const std::string & name, Tokenizer & tokenizer)
{
  Token lparen = tokenizer.nextToken();
  if (lparen.type != Token::LPAREN)
    mooseError("Expected opening parenthesis after function " + name);
  
  std::vector<NodePtr> args;
  
  Token next = tokenizer.peekToken();
  if (next.type != Token::RPAREN)
  {
    do
    {
      args.push_back(parseExpression(tokenizer));
      next = tokenizer.peekToken();
      if (next.type == Token::COMMA)
        tokenizer.nextToken(); // consume comma
    }
    while (next.type == Token::COMMA);
  }
  
  Token rparen = tokenizer.nextToken();
  if (rparen.type != Token::RPAREN)
    mooseError("Expected closing parenthesis after function " + name);
  
  // Handle built-in functions
  if (name == "grad" || name == "gradient")
    return parseGradient(args);
  else if (name == "div" || name == "divergence")
    return parseDivergence(args);
  else if (name == "laplacian" || name == "laplace")
    return parseLaplacian(args);
  else if (name == "hessian")
    return parseHessian(args);
  else if (name == "curl")
    return parseCurl(args);
  else if (name == "dot")
    return parseDot(args);
  else if (name == "cross")
    return parseCross(args);
  else if (name == "norm" || name == "magnitude")
    return parseNorm(args);
  else if (name == "normalize")
    return parseNormalize(args);
  else if (name == "trace" || name == "tr")
    return parseTrace(args);
  else if (name == "det" || name == "determinant")
    return parseDet(args);
  else if (name == "inv" || name == "inverse")
    return parseInv(args);
  else if (name == "transpose" || name == "trans")
    return parseTranspose(args);
  else if (name == "sym" || name == "symmetric")
    return parseSym(args);
  else if (name == "skew")
    return parseSkew(args);
  else if (name == "dev" || name == "deviatoric")
    return parseDev(args);
  else if (name == "contract")
    return parseContract(args);
  else if (name == "outer")
    return parseOuter(args);
  else if (name == "vec" || name == "vector")
    return parseVec(args);
  else if (name == "exp")
    return parseExp(args);
  else if (name == "log" || name == "ln")
    return parseLog(args);
  else if (name == "sin")
    return parseSin(args);
  else if (name == "cos")
    return parseCos(args);
  else if (name == "tan")
    return parseTan(args);
  else if (name == "sqrt")
    return parseSqrt(args);
  else if (name == "abs")
    return parseAbs(args);
  else if (name == "pow")
    return parsePow(args);
  else if (name == "W" || name == "doublewell")
    return parseDoubleWell(args);
  else if (_custom_functions.count(name))
    return expandCustomFunction(name, args);
  else
    return function(name, args);
}

inline NodePtr StringExpressionParser::parseGradient(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("gradient() requires exactly 1 argument");
  return grad(args[0], _dim);
}

inline NodePtr StringExpressionParser::parseDivergence(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("divergence() requires exactly 1 argument");
  return div(args[0]);
}

inline NodePtr StringExpressionParser::parseLaplacian(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("laplacian() requires exactly 1 argument");
  return laplacian(args[0]);
}

inline NodePtr StringExpressionParser::parseHessian(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("hessian() requires exactly 1 argument");
  return grad(grad(args[0], _dim), _dim);
}

inline NodePtr StringExpressionParser::parseDot(const std::vector<NodePtr> & args)
{
  if (args.size() != 2)
    mooseError("dot() requires exactly 2 arguments");
  return moose::automatic_weak_form::dot(args[0], args[1]);
}

inline NodePtr StringExpressionParser::parseNorm(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("norm() requires exactly 1 argument");
  return norm(args[0]);
}

inline NodePtr StringExpressionParser::parseTrace(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("trace() requires exactly 1 argument");
  return trace(args[0]);
}

inline NodePtr StringExpressionParser::parseDet(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("det() requires exactly 1 argument");
  return det(args[0]);
}

inline NodePtr StringExpressionParser::parseInv(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("inv() requires exactly 1 argument");
  return inv(args[0]);
}

inline NodePtr StringExpressionParser::parseTranspose(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("transpose() requires exactly 1 argument");
  return transpose(args[0]);
}

inline NodePtr StringExpressionParser::parseSym(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("sym() requires exactly 1 argument");
  return sym(args[0]);
}

inline NodePtr StringExpressionParser::parsePow(const std::vector<NodePtr> & args)
{
  if (args.size() != 2)
    mooseError("pow() requires exactly 2 arguments");
  return power(args[0], args[1]);
}

inline NodePtr StringExpressionParser::parseDoubleWell(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("W() requires exactly 1 argument");
  
  // W(c) = (c^2 - 1)^2
  auto c = args[0];
  auto c2 = multiply(c, c);
  auto c2_minus_1 = subtract(c2, constant(1.0));
  return multiply(c2_minus_1, c2_minus_1);
}

inline NodePtr StringExpressionParser::parseExp(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("exp() requires exactly 1 argument");
  return function("exp", args);
}

inline NodePtr StringExpressionParser::parseLog(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("log() requires exactly 1 argument");
  return function("log", args);
}

inline NodePtr StringExpressionParser::parseSin(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("sin() requires exactly 1 argument");
  return function("sin", args);
}

inline NodePtr StringExpressionParser::parseCos(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("cos() requires exactly 1 argument");
  return function("cos", args);
}

inline NodePtr StringExpressionParser::parseSqrt(const std::vector<NodePtr> & args)
{
  if (args.size() != 1)
    mooseError("sqrt() requires exactly 1 argument");
  return power(args[0], constant(0.5));
}

inline NodePtr StringExpressionParser::parseVec(const std::vector<NodePtr> & args)
{
  if (args.size() < 2 || args.size() > 3)
    mooseError("vec() requires 2 or 3 arguments (for 2D or 3D vectors)");
  
  // Create a vector from scalar components
  if (args.size() == 2)
  {
    // 2D vector
    return vec2(args[0], args[1]);
  }
  else
  {
    // 3D vector
    return vec3(args[0], args[1], args[2]);
  }
}

// Tokenizer implementation

inline StringExpressionParser::Token StringExpressionParser::Tokenizer::nextToken()
{
  skipWhitespace();
  
  if (_pos >= _expr.length())
  {
    _last_token = Token(Token::END);
    return _last_token;
  }
  
  char ch = _expr[_pos];
  
  if (std::isdigit(ch) || ch == '.')
    _last_token = readNumber();
  else if (std::isalpha(ch) || ch == '_')
    _last_token = readIdentifier();
  else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')
    _last_token = readOperator();
  else if (ch == '(')
  {
    _pos++;
    _last_token = Token(Token::LPAREN, "(");
  }
  else if (ch == ')')
  {
    _pos++;
    _last_token = Token(Token::RPAREN, ")");
  }
  else if (ch == '[')
  {
    _pos++;
    _last_token = Token(Token::LBRACKET, "[");
  }
  else if (ch == ']')
  {
    _pos++;
    _last_token = Token(Token::RBRACKET, "]");
  }
  else if (ch == ',')
  {
    _pos++;
    _last_token = Token(Token::COMMA, ",");
  }
  else if (ch == '=')
  {
    _pos++;
    _last_token = Token(Token::EQUALS, "=");
  }
  else
    mooseError("Unexpected character: " + std::string(1, ch));
  
  return _last_token;
}

inline StringExpressionParser::Token StringExpressionParser::Tokenizer::peekToken()
{
  size_t saved_pos = _pos;
  Token token = nextToken();
  _pos = saved_pos;
  return token;
}

inline void StringExpressionParser::Tokenizer::skipWhitespace()
{
  while (_pos < _expr.length() && std::isspace(_expr[_pos]))
    _pos++;
}

inline StringExpressionParser::Token StringExpressionParser::Tokenizer::readNumber()
{
  size_t start = _pos;
  bool has_dot = false;
  bool has_exp = false;
  
  while (_pos < _expr.length())
  {
    char ch = _expr[_pos];
    if (std::isdigit(ch))
    {
      _pos++;
    }
    else if (ch == '.' && !has_dot && !has_exp)
    {
      has_dot = true;
      _pos++;
    }
    else if ((ch == 'e' || ch == 'E') && !has_exp)
    {
      has_exp = true;
      _pos++;
      if (_pos < _expr.length() && (_expr[_pos] == '+' || _expr[_pos] == '-'))
        _pos++;
    }
    else
    {
      break;
    }
  }
  
  std::string num_str = _expr.substr(start, _pos - start);
  Real value = std::stod(num_str);
  return Token(Token::NUMBER, num_str, value);
}

inline StringExpressionParser::Token StringExpressionParser::Tokenizer::readIdentifier()
{
  size_t start = _pos;
  
  while (_pos < _expr.length() && (std::isalnum(_expr[_pos]) || _expr[_pos] == '_'))
    _pos++;
  
  std::string id = _expr.substr(start, _pos - start);
  return Token(Token::IDENTIFIER, id);
}

inline StringExpressionParser::Token StringExpressionParser::Tokenizer::readOperator()
{
  std::string op(1, _expr[_pos++]);
  return Token(Token::OPERATOR, op);
}

}
}