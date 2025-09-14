#pragma once

#include "MooseAST.h"
#include "StringExpressionParser.h"
#include "MooseObject.h"
#include "InputParameters.h"
#include <sstream>
#include <stack>

namespace moose
{
namespace automatic_weak_form
{

class MooseExpressionBuilder
{
public:
  MooseExpressionBuilder(unsigned int dim = 3) : _dim(dim) {}
  
  NodePtr parseExpression(const std::string & expr_str);
  NodePtr parseExpression(const std::string & expr_str, const std::map<std::string, Real> & params);
  
  void setParameter(const std::string & name, Real value) { _parameters[name] = value; }
  
  NodePtr buildFromParameters(const InputParameters & params);
  
  NodePtr scalar(Real value) { return constant(value); }
  NodePtr scalar(const std::string & name) { return variable(name, ScalarShape{}); }
  
  NodePtr vector(const RealVectorValue & value) { return constant(value, _dim); }
  NodePtr vector(const std::string & name) { return variable(name, VectorShape{_dim}); }
  
  NodePtr tensor(const RankTwoTensor & value) { return constant(value, _dim); }
  NodePtr tensor(const std::string & name) { return variable(name, TensorShape{_dim}); }
  
  NodePtr field(const std::string & name, const Shape & shape = ScalarShape{})
  {
    return fieldVariable(name, shape);
  }
  
  NodePtr test(const std::string & var_name, bool gradient = false)
  {
    return testFunction(var_name, gradient);
  }
  
  NodePtr phi(const std::string & var_name, bool gradient = false)
  {
    return shapeFunction(var_name, gradient);
  }
  
  NodePtr gradField(const std::string & name)
  {
    auto field_var = fieldVariable(name, ScalarShape{});
    return grad(field_var, _dim);
  }
  
  NodePtr hessianField(const std::string & name)
  {
    auto grad_field = gradField(name);
    return grad(grad_field, _dim);
  }
  
  NodePtr divField(const std::string & name)
  {
    auto field_var = fieldVariable(name, VectorShape{_dim});
    return div(field_var);
  }
  
  NodePtr curlField(const std::string & name)
  {
    auto field_var = fieldVariable(name, VectorShape{_dim});
    return curl(field_var, _dim);
  }
  
  NodePtr strain(const std::string & disp_name)
  {
    auto u = fieldVariable(disp_name, VectorShape{_dim});
    auto grad_u = grad(u, _dim);
    return sym(grad_u);
  }
  
  NodePtr deformationGradient(const std::string & disp_name)
  {
    auto u = fieldVariable(disp_name, VectorShape{_dim});
    auto grad_u = grad(u, _dim);
    auto I = identityTensor();
    return add(I, grad_u);
  }
  
  NodePtr rightCauchyGreen(const std::string & disp_name)
  {
    auto F = deformationGradient(disp_name);
    auto Ft = transpose(F);
    return multiply(Ft, F);
  }
  
  NodePtr greenLagrangeStrain(const std::string & disp_name)
  {
    auto C = rightCauchyGreen(disp_name);
    auto I = identityTensor();
    auto CminusI = subtract(C, I);
    return multiply(constant(0.5), CminusI);
  }
  
  NodePtr doubleWell(const NodePtr & c, Real height = 1.0)
  {
    auto c2 = multiply(c, c);
    auto one = constant(1.0);
    auto c2_minus_1 = subtract(c2, one);
    auto well = multiply(c2_minus_1, c2_minus_1);
    return multiply(constant(height), well);
  }
  
  NodePtr obstacle(const NodePtr & c, Real lower = -1.0, Real upper = 1.0)
  {
    auto c_lower = subtract(c, constant(lower));
    auto upper_c = subtract(constant(upper), c);
    auto penalty = function("obstacle_penalty", {c_lower, upper_c});
    return penalty;
  }
  
  NodePtr elasticEnergy(const NodePtr & strain_tensor, Real lambda, Real mu)
  {
    auto tr_eps = trace(strain_tensor);
    auto tr_eps_sq = multiply(tr_eps, tr_eps);
    auto eps_contract = contract(strain_tensor, strain_tensor);
    
    auto bulk_term = multiply(constant(0.5 * lambda), tr_eps_sq);
    auto shear_term = multiply(constant(mu), eps_contract);
    
    return add(bulk_term, shear_term);
  }
  
  NodePtr neoHookean(const NodePtr & F, Real mu, Real lambda)
  {
    auto C = multiply(transpose(F), F);
    auto I1 = trace(C);
    auto J = det(F);
    auto lnJ = function("log", {J});
    
    auto shear = multiply(constant(mu/2), subtract(I1, constant(3.0)));
    auto bulk = multiply(constant(lambda/2), multiply(lnJ, lnJ));
    auto volume = multiply(constant(-mu), lnJ);
    
    return add(add(shear, bulk), volume);
  }
  
  NodePtr surfaceEnergy(const NodePtr & phi_field, Real gamma_0)
  {
    auto grad_phi = grad(phi_field, _dim);
    auto mag_grad = norm(grad_phi);
    return multiply(constant(gamma_0), mag_grad);
  }
  
  NodePtr anisotropicSurface(const NodePtr & phi_field, 
                              const std::function<NodePtr(const NodePtr &)> & gamma_func)
  {
    auto grad_phi = grad(phi_field, _dim);
    auto mag_grad = norm(grad_phi);
    auto normal = normalize(grad_phi);
    auto gamma = gamma_func(normal);
    return multiply(gamma, mag_grad);
  }
  
  NodePtr allencahn(const NodePtr & c_field, const NodePtr & mobility, Real kappa = 1.0)
  {
    auto dw_dc = function("dW_dc", {c_field});
    auto grad_c = grad(c_field, _dim);
    auto laplace_c = laplacian(c_field);
    
    auto bulk = multiply(mobility, dw_dc);
    auto interface = multiply(multiply(mobility, constant(-kappa)), laplace_c);
    
    return add(bulk, interface);
  }
  
  NodePtr cahnHilliard(const NodePtr & /*c_field*/, const NodePtr & mu_field, 
                        const NodePtr & mobility)
  {
    auto grad_mu = grad(mu_field, _dim);
    return multiply(negate(mobility), div(grad_mu));
  }
  
  NodePtr fourthOrderRegularization(const NodePtr & c_field, Real lambda)
  {
    auto hess_c = hessianField(c_field->toString());
    auto frobenius = contract(hess_c, hess_c);
    return multiply(constant(0.5 * lambda), frobenius);
  }
  
private:
  unsigned int _dim;
  std::map<std::string, Real> _parameters;
  
  NodePtr identityTensor()
  {
    RankTwoTensor I;
    I.zero();
    I.addIa(1.0);
    return constant(I, _dim);
  }
  
  struct Token
  {
    enum Type { NUMBER, IDENTIFIER, OPERATOR, LPAREN, RPAREN, COMMA, END };
    Type type;
    std::string value;
    Real number;
  };
  
  class Tokenizer
  {
  public:
    Tokenizer(const std::string & expr) : _expr(expr), _pos(0) {}
    
    Token nextToken();
    
  private:
    std::string _expr;
    size_t _pos;
    
    void skipWhitespace();
    Token readNumber();
    Token readIdentifier();
  };
  
  class Parser
  {
  public:
    Parser(Tokenizer & tokenizer, MooseExpressionBuilder & builder)
      : _tokenizer(tokenizer), _builder(builder) {}
    
    NodePtr parse();
    
  private:
    Tokenizer & _tokenizer;
    MooseExpressionBuilder & _builder;
    Token _current;
    
    void advance() { _current = _tokenizer.nextToken(); }
    
    NodePtr parseExpression();
    NodePtr parseTerm();
    NodePtr parseFactor();
    NodePtr parsePrimary();
    NodePtr parseFunction(const std::string & name);
  };
};

inline NodePtr MooseExpressionBuilder::parseExpression(const std::string & expr_str)
{
  StringExpressionParser expr_parser(_dim);
  
  // Set any stored parameters
  for (const auto & [name, value] : _parameters)
    expr_parser.setParameter(name, value);
  
  return expr_parser.parse(expr_str);
}

inline NodePtr MooseExpressionBuilder::parseExpression(const std::string & expr_str,
                                                      const std::map<std::string, Real> & params)
{
  StringExpressionParser expr_parser(_dim);
  
  // Set parameters
  for (const auto & [name, value] : params)
    expr_parser.setParameter(name, value);
  
  return expr_parser.parse(expr_str);
}

inline NodePtr MooseExpressionBuilder::buildFromParameters(const InputParameters & params)
{
  if (params.isParamValid("energy_expression"))
  {
    std::string expr = params.get<std::string>("energy_expression");
    
    // Check if we have parameters to substitute
    if (params.isParamValid("parameters"))
    {
      auto param_map = params.get<std::map<std::string, Real>>("parameters");
      return parseExpression(expr, param_map);
    }
    else
    {
      return parseExpression(expr);
    }
  }
  
  if (params.isParamValid("energy_type"))
  {
    std::string type = params.get<std::string>("energy_type");
    
    if (type == "double_well")
    {
      auto c = field(params.get<std::string>("variable"));
      Real height = params.isParamValid("well_height") ? 
                    params.get<Real>("well_height") : 1.0;
      return doubleWell(c, height);
    }
    else if (type == "elastic")
    {
      auto u = field(params.get<std::string>("displacement"));
      auto eps = strain(u->toString());
      Real lambda = params.get<Real>("lambda");
      Real mu = params.get<Real>("mu");
      return elasticEnergy(eps, lambda, mu);
    }
    else if (type == "neo_hookean")
    {
      auto u = field(params.get<std::string>("displacement"));
      auto F = deformationGradient(u->toString());
      Real lambda = params.get<Real>("lambda");
      Real mu = params.get<Real>("mu");
      return neoHookean(F, mu, lambda);
    }
    else if (type == "surface")
    {
      auto phi = field(params.get<std::string>("order_parameter"));
      Real gamma = params.get<Real>("surface_energy");
      return surfaceEnergy(phi, gamma);
    }
    else if (type == "cahn_hilliard")
    {
      auto c = field(params.get<std::string>("concentration"));
      Real kappa = params.get<Real>("gradient_coefficient");
      
      auto grad_c = grad(c, _dim);
      auto grad_term = multiply(constant(0.5 * kappa), dot(grad_c, grad_c));
      auto bulk_term = doubleWell(c);
      
      return add(bulk_term, grad_term);
    }
    else if (type == "fourth_order")
    {
      auto c = field(params.get<std::string>("variable"));
      Real kappa = params.get<Real>("gradient_coefficient");
      Real lambda = params.get<Real>("fourth_order_coefficient");
      
      auto grad_c = grad(c, _dim);
      auto grad_term = multiply(constant(0.5 * kappa), dot(grad_c, grad_c));
      auto bulk_term = doubleWell(c);
      auto fourth_term = fourthOrderRegularization(c, lambda);
      
      return add(add(bulk_term, grad_term), fourth_term);
    }
  }
  
  mooseError("Unable to build expression from parameters");
  return nullptr;
}

inline MooseExpressionBuilder::Token MooseExpressionBuilder::Tokenizer::nextToken()
{
  skipWhitespace();
  
  if (_pos >= _expr.length())
    return {Token::END, "", 0};
  
  char ch = _expr[_pos];
  
  if (std::isdigit(ch) || ch == '.')
    return readNumber();
  
  if (std::isalpha(ch) || ch == '_')
    return readIdentifier();
  
  if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')
  {
    _pos++;
    return {Token::OPERATOR, std::string(1, ch), 0};
  }
  
  if (ch == '(')
  {
    _pos++;
    return {Token::LPAREN, "(", 0};
  }
  
  if (ch == ')')
  {
    _pos++;
    return {Token::RPAREN, ")", 0};
  }
  
  if (ch == ',')
  {
    _pos++;
    return {Token::COMMA, ",", 0};
  }
  
  mooseError("Unexpected character in expression: " + std::string(1, ch));
  return {Token::END, "", 0};
}

inline void MooseExpressionBuilder::Tokenizer::skipWhitespace()
{
  while (_pos < _expr.length() && std::isspace(_expr[_pos]))
    _pos++;
}

inline MooseExpressionBuilder::Token MooseExpressionBuilder::Tokenizer::readNumber()
{
  size_t start = _pos;
  bool has_dot = false;
  
  while (_pos < _expr.length())
  {
    char ch = _expr[_pos];
    if (std::isdigit(ch))
    {
      _pos++;
    }
    else if (ch == '.' && !has_dot)
    {
      has_dot = true;
      _pos++;
    }
    else if (ch == 'e' || ch == 'E')
    {
      _pos++;
      if (_pos < _expr.length() && (_expr[_pos] == '+' || _expr[_pos] == '-'))
        _pos++;
      while (_pos < _expr.length() && std::isdigit(_expr[_pos]))
        _pos++;
      break;
    }
    else
    {
      break;
    }
  }
  
  std::string num_str = _expr.substr(start, _pos - start);
  Real value = std::stod(num_str);
  return {Token::NUMBER, num_str, value};
}

inline MooseExpressionBuilder::Token MooseExpressionBuilder::Tokenizer::readIdentifier()
{
  size_t start = _pos;
  
  while (_pos < _expr.length())
  {
    char ch = _expr[_pos];
    if (std::isalnum(ch) || ch == '_')
      _pos++;
    else
      break;
  }
  
  std::string id = _expr.substr(start, _pos - start);
  return {Token::IDENTIFIER, id, 0};
}

inline NodePtr MooseExpressionBuilder::Parser::parse()
{
  advance();
  return parseExpression();
}

inline NodePtr MooseExpressionBuilder::Parser::parseExpression()
{
  NodePtr left = parseTerm();
  
  while (_current.type == Token::OPERATOR && 
         (_current.value == "+" || _current.value == "-"))
  {
    std::string op = _current.value;
    advance();
    NodePtr right = parseTerm();
    
    if (op == "+")
      left = add(left, right);
    else
      left = subtract(left, right);
  }
  
  return left;
}

inline NodePtr MooseExpressionBuilder::Parser::parseTerm()
{
  NodePtr left = parseFactor();
  
  while (_current.type == Token::OPERATOR && 
         (_current.value == "*" || _current.value == "/"))
  {
    std::string op = _current.value;
    advance();
    NodePtr right = parseFactor();
    
    if (op == "*")
      left = multiply(left, right);
    else
      left = divide(left, right);
  }
  
  return left;
}

inline NodePtr MooseExpressionBuilder::Parser::parseFactor()
{
  if (_current.type == Token::OPERATOR && _current.value == "-")
  {
    advance();
    return negate(parseFactor());
  }
  
  return parsePrimary();
}

inline NodePtr MooseExpressionBuilder::Parser::parsePrimary()
{
  if (_current.type == Token::NUMBER)
  {
    NodePtr node = constant(_current.number);
    advance();
    return node;
  }
  
  if (_current.type == Token::IDENTIFIER)
  {
    std::string name = _current.value;
    advance();
    
    if (_current.type == Token::LPAREN)
    {
      return parseFunction(name);
    }
    else
    {
      return variable(name, ScalarShape{});
    }
  }
  
  if (_current.type == Token::LPAREN)
  {
    advance();
    NodePtr expr = parseExpression();
    if (_current.type != Token::RPAREN)
      mooseError("Expected closing parenthesis");
    advance();
    return expr;
  }
  
  mooseError("Unexpected token in expression");
  return nullptr;
}

inline NodePtr MooseExpressionBuilder::Parser::parseFunction(const std::string & name)
{
  advance();
  
  std::vector<NodePtr> args;
  
  if (_current.type != Token::RPAREN)
  {
    args.push_back(parseExpression());
    
    while (_current.type == Token::COMMA)
    {
      advance();
      args.push_back(parseExpression());
    }
  }
  
  if (_current.type != Token::RPAREN)
    mooseError("Expected closing parenthesis in function call");
  advance();
  
  if (name == "grad" && args.size() == 1)
    return grad(args[0], _builder._dim);
  else if (name == "div" && args.size() == 1)
    return div(args[0]);
  else if (name == "laplacian" && args.size() == 1)
    return laplacian(args[0]);
  else if (name == "curl" && args.size() == 1)
    return curl(args[0], _builder._dim);
  else if (name == "dot" && args.size() == 2)
    return dot(args[0], args[1]);
  else if (name == "cross" && args.size() == 2)
    return cross(args[0], args[1]);
  else if (name == "outer" && args.size() == 2)
    return outer(args[0], args[1]);
  else if (name == "contract" && args.size() == 2)
    return contract(args[0], args[1]);
  else if (name == "norm" && args.size() == 1)
    return norm(args[0]);
  else if (name == "normalize" && args.size() == 1)
    return normalize(args[0]);
  else if (name == "trace" && args.size() == 1)
    return trace(args[0]);
  else if (name == "det" && args.size() == 1)
    return det(args[0]);
  else if (name == "inv" && args.size() == 1)
    return inv(args[0]);
  else if (name == "transpose" && args.size() == 1)
    return transpose(args[0]);
  else if (name == "sym" && args.size() == 1)
    return sym(args[0]);
  else if (name == "skew" && args.size() == 1)
    return skew(args[0]);
  else if (name == "dev" && args.size() == 1)
    return dev(args[0]);
  else if (name == "pow" && args.size() == 2)
    return power(args[0], args[1]);
  else
    return function(name, args);
}

}
}