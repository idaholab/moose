//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctionParser.h"

namespace Moose::Kokkos
{

Lexer::Token
Lexer::next()
{
  while (isSpace(get(_pos)))
    ++_pos;

  const char c = get(_pos);

  if (c == '\0')
    return {TokenType::END, "", _pos};

  // Parse number (e.g., 123, 3.14, .5, 1e10)
  if (isDigit(c) || (c == '.' && isDigit(get(_pos + 1))))
  {
    unsigned int start = _pos;

    while (isDigit(get(_pos)))
      ++_pos;

    if (get(_pos) == '.')
    {
      ++_pos;

      while (isDigit(get(_pos)))
        ++_pos;
    }

    if (get(_pos) == 'e' || get(_pos) == 'E')
    {
      unsigned int epos = _pos + 1;

      if (get(epos) == '+' || get(epos) == '-')
        ++epos;

      bool has_digit = false;

      while (isDigit(get(epos)))
      {
        has_digit = true;
        ++epos;
      }

      if (has_digit)
        _pos = epos;
    }

    auto text = _expression.substr(start, _pos - start);
    return {TokenType::NUMBER, text, _pos, std::stod(text)};
  }

  // Parse identifier ([A-Za-z_][A-Za-z0-9_]*)
  if (isAlpha(c) || c == '_')
  {
    auto start = _pos++;

    while (isAlnum(get(_pos)) || get(_pos) == '_')
      ++_pos;

    return {TokenType::IDENTIFIER, _expression.substr(start, _pos - start), _pos};
  }

  // Parse parenthesis and comma
  if (c == '(')
  {
    ++_pos;
    return {TokenType::LEFT, "(", _pos};
  }
  if (c == ')')
  {
    ++_pos;
    return {TokenType::RIGHT, ")", _pos};
  }
  if (c == ',')
  {
    ++_pos;
    return {TokenType::COMMA, ",", _pos};
  }

  // Parse operators (longest match first)
  for (const auto & op : _operators)
    if (!_expression.compare(_pos, op.size(), op))
    {
      _pos += op.size();
      return {TokenType::OPERATOR, op, _pos};
    }

  lexerError(std::string("unexpected character '") + c + "'.");
}

void
Lexer::lexerError(const std::string & message)
{
  std::string arrow(_pos + 1, ' ');
  arrow.back() = '^';

  mooseError("Kokkos parsed function error: ", message, "\n", _expression, "\n", arrow);
}

std::unique_ptr<PrattParser::SyntaxBase>
PrattParser::parseExpression(unsigned int min_binding_power)
{
  auto lhs = parsePrimary();

  while (_token.type == Lexer::TokenType::OPERATOR && _infix_operators.count(_token.expression))
  {
    auto [bp, right_assoc] = infixInfo(_token.expression);

    if (bp < min_binding_power)
      break;

    std::string op = _token.expression;

    advance();

    auto rhs = parseExpression(right_assoc ? bp : bp + 1);

    lhs = std::make_unique<SyntaxBinary>(_token.pos, op, std::move(lhs), std::move(rhs));
  }

  return lhs;
}

std::unique_ptr<PrattParser::SyntaxBase>
PrattParser::parsePrimary()
{
  std::unique_ptr<SyntaxBase> lhs;

  auto pos = _token.pos;

  // Unary
  if (_token.type == Lexer::TokenType::OPERATOR && _prefix_operators.count(_token.expression))
  {
    auto op = _token.expression;
    advance();
    auto rhs = parseExpression(_prefix_binding_power);
    lhs = std::make_unique<SyntaxUnary>(pos, op, std::move(rhs));
  }
  // Number
  else if (_token.type == Lexer::TokenType::NUMBER)
  {
    auto v = _token.value;
    advance();
    lhs = std::make_unique<SyntaxNumber>(pos, v);
  }
  // Identifier
  else if (_token.type == Lexer::TokenType::IDENTIFIER)
  {
    auto name = _token.expression;
    advance();
    lhs = std::make_unique<SyntaxIdentifier>(pos, name);
  }
  // Parenthesis
  else if (_token.type == Lexer::TokenType::LEFT)
  {
    advance();
    lhs = parseExpression(0);
    if (_token.type != Lexer::TokenType::RIGHT)
      parserError("expected ')'.");
    advance();
  }
  else
    parserError("expected an expression.");

  // Function call
  while (_token.type == Lexer::TokenType::LEFT)
  {
    advance();
    std::vector<std::unique_ptr<SyntaxBase>> args;
    if (_token.type != Lexer::TokenType::RIGHT)
    {
      while (true)
      {
        auto arg = parseExpression(0);
        args.push_back(std::move(arg));
        if (_token.type == Lexer::TokenType::COMMA)
        {
          advance();
          continue;
        }
        break;
      }
    }
    if (_token.type != Lexer::TokenType::RIGHT)
      parserError("expected ')'.");
    advance();

    lhs = std::make_unique<SyntaxCall>(pos, std::move(lhs), std::move(args));
  }

  return lhs;
}

void
PrattParser::verify()
{
  // Extra right parentheses will cause early completion of parsing
  if (_token.type != Lexer::TokenType::END)
    parserError("extra ')'.");
}

void
PrattParser::parserError(const std::string & message)
{
  std::string arrow(_token.pos, ' ');
  arrow.back() = '^';

  mooseError("Kokkos parsed function error: ", message, "\n", _lexer.expression(), "\n", arrow);
}

RPNBuilder::RPNBuilder(const std::string & expression) : _parser(expression)
{
  _ast = _parser.parseExpression();

  _parser.verify();
}

void
RPNBuilder::build(const PrattParser::SyntaxBase * syntax)
{
  if (auto n = dynamic_cast<const PrattParser::SyntaxNumber *>(syntax))
    _rpn.push_back({Opcode::NUM, std::to_string(n->value), addNumber(n->value)});
  else if (auto id = dynamic_cast<const PrattParser::SyntaxIdentifier *>(syntax))
  {
    if (_function_opcode_map.count(id->name))
      builderError(id, "variable name '" + id->name + "' is reserved for function.");

    _rpn.push_back({Opcode::VAR, id->name, addVariable(id->name)});
  }
  else if (auto u = dynamic_cast<const PrattParser::SyntaxUnary *>(syntax))
  {
    build(u->rhs.get());

    if (u->op == "-")
      _rpn.push_back({Opcode::NEG, u->op});
    else if (u->op == "!")
    {
      _rpn.push_back({Opcode::NUM, "0.0", addNumber(0.0)});
      _rpn.push_back({Opcode::NUM, "1.0", addNumber(1.0)});
      _rpn.push_back({Opcode::IF, "!"});
    }
    else
      builderError(u, "unsupported unary operator '" + u->op + "'.");
  }
  else if (auto b = dynamic_cast<const PrattParser::SyntaxBinary *>(syntax))
  {
    build(b->lhs.get());
    build(b->rhs.get());

    auto it = _binary_opcode_map.find(b->op);
    if (it == _binary_opcode_map.end())
      builderError(b, "unsupported binary operator '" + b->op + "'.");

    _rpn.push_back({it->second, b->op});
  }
  else if (auto c = dynamic_cast<const PrattParser::SyntaxCall *>(syntax))
  {
    auto callee = dynamic_cast<const PrattParser::SyntaxIdentifier *>(c->callee.get());
    if (!callee)
      builderError(c, "invalid function call syntax.");

    auto it = _function_opcode_map.find(callee->name);
    if (it == _function_opcode_map.end())
      builderError(c, "unsupported function '" + callee->name + "'.");

    if (c->args.size() != it->second.second)
      builderError(
          c, callee->name + "() expects " + std::to_string(it->second.second) + " argument(s).");

    for (auto & arg : c->args)
      build(arg.get());

    _rpn.push_back({it->second.first, callee->name});
  }
}

void
RPNBuilder::printAST(const ConsoleStream & console,
                     const PrattParser::SyntaxBase * syntax,
                     unsigned int indent)
{
  auto pad = [&](unsigned int indent)
  {
    for (unsigned int i = 0; i < indent; ++i)
      console << " ";
  };

  if (auto n = dynamic_cast<const PrattParser::SyntaxNumber *>(syntax))
  {
    pad(indent);
    console << "Number (" << n->value << ")" << std::endl;
  }
  else if (auto v = dynamic_cast<const PrattParser::SyntaxIdentifier *>(syntax))
  {
    pad(indent);
    console << "Identifier (" << v->name << ")" << std::endl;
  }
  else if (auto u = dynamic_cast<const PrattParser::SyntaxUnary *>(syntax))
  {
    pad(indent);
    console << "Unary (" << u->op << ")" << std::endl;
    printAST(console, u->rhs.get(), indent + 2);
  }
  else if (auto b = dynamic_cast<const PrattParser::SyntaxBinary *>(syntax))
  {
    pad(indent);
    console << "Binary (" << b->op << ")" << std::endl;
    printAST(console, b->lhs.get(), indent + 2);
    printAST(console, b->rhs.get(), indent + 2);
  }
  else if (auto c = dynamic_cast<const PrattParser::SyntaxCall *>(syntax))
  {
    pad(indent);
    console << "Call (" << std::endl;
    pad(indent + 2);
    console << "Callee:" << std::endl;
    printAST(console, c->callee.get(), indent + 4);
    pad(indent + 2);
    console << "Arguments:" << std::endl;
    for (const auto & arg : c->args)
      printAST(console, arg.get(), indent + 4);
    pad(indent);
    console << ")" << std::endl;
  }
}

void
RPNBuilder::printRPN(const ConsoleStream & console)
{
  for (const auto & instruction : _rpn)
    console << instruction.text << " ";
  console << std::endl;
}

unsigned int
RPNBuilder::addNumber(Real number)
{
  _numbers.push_back(number);
  return _numbers.size() - 1;
}

unsigned int
RPNBuilder::addVariable(const std::string & name)
{
  auto it = _variables.find(name);
  if (it != _variables.end())
    return it->second;

  unsigned int idx = _variables.size();
  _variables[name] = idx;
  return idx;
}

void
RPNBuilder::builderError(const PrattParser::SyntaxBase * syntax, const std::string & message)
{
  std::string arrow(syntax->pos, ' ');
  arrow.back() = '^';

  mooseError("Kokkos parsed function error: ", message, "\n", _parser.expression(), "\n", arrow);
}

} // namespace Moose::Kokkos
