//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"

#include "ConsoleStreamInterface.h"

namespace Moose::Kokkos
{

/**
 * Lexer for parsed function expression
 */
class Lexer
{
public:
  /**
   * Constructor
   * @param expression The function expression
   */
  Lexer(const std::string & expression) : _expression(expression) {}

  /**
   * Get the function expression
   * @returns The function expression
   */
  const std::string & expression() { return _expression; }

  /**
   * Unit token types
   */
  enum class TokenType
  {
    // End of expression
    END,
    // Number
    NUMBER,
    // Identifier (variable or function)
    IDENTIFIER,
    // Operator
    OPERATOR,
    // Left parenthesis
    LEFT,
    // Right parenthesis
    RIGHT,
    // Comma
    COMMA
  };

  /**
   * Unit token
   */
  struct Token
  {
    // Token type
    TokenType type;
    // Token string
    std::string expression;
    // Token position
    unsigned int pos;
    // Value for number token
    double value = 0;
  };

  /**
   * Get the next token
   */
  Token next();

private:
  /**
   * Function expression
   */
  const std::string _expression;
  /**
   * Position pointer
   */
  unsigned int _pos = 0;
  /**
   * Supported operators
   */
  static inline const std::vector<std::string> _operators = {
      "!=", "<=", ">=", "|", "&", "^", "%", "/", "*", "+", "-", "<", ">", "!", "="};

  /**
   * Get the character of the expression at a position
   * @param pos The position
   * @returns The character of the expression at the position, NULL terminator if the position is
   * after the end of expression
   */
  char get(unsigned int pos) { return pos < _expression.size() ? _expression[pos] : '\0'; }

  /**
   * Shortcuts to C locale functions
   */
  ///{@
  bool isSpace(const char c) { return std::isspace(static_cast<unsigned char>(c)); }
  bool isDigit(const char c) { return std::isdigit(static_cast<unsigned char>(c)); }
  bool isAlpha(const char c) { return std::isalpha(static_cast<unsigned char>(c)); }
  bool isAlnum(const char c) { return std::isalnum(static_cast<unsigned char>(c)); }
  ///@}

  /**
   * Print a pretty error showing the position of error
   * @param message The error message
   */
  [[noreturn]] void lexerError(const std::string & message);
};

/**
 * Pratt parser for parsed function expression
 */
class PrattParser
{
public:
  /**
   * Constructor
   * @param expression The function expression
   */
  PrattParser(const std::string & expression) : _lexer(expression) { advance(); }

  /**
   * Get the function expression
   * @returns The function expression
   */
  const std::string & expression() { return _lexer.expression(); }

  /**
   * Abstract Syntax Tree (AST) entry
   */
  ///@{
  struct SyntaxBase
  {
    unsigned int pos;
    SyntaxBase(unsigned int p) : pos(p) {}
    virtual ~SyntaxBase() = default;
  };

  struct SyntaxNumber : SyntaxBase
  {
    Real value;
    SyntaxNumber(unsigned int p, Real v) : SyntaxBase(p), value(v) {}
  };

  struct SyntaxIdentifier : SyntaxBase
  {
    std::string name;
    SyntaxIdentifier(unsigned int p, std::string & n) : SyntaxBase(p), name(n) {}
  };

  struct SyntaxUnary : SyntaxBase
  {
    std::string op;
    std::unique_ptr<SyntaxBase> rhs;
    SyntaxUnary(unsigned int p, std::string & o, std::unique_ptr<SyntaxBase> r)
      : SyntaxBase(p), op(o), rhs(std::move(r))
    {
    }
  };

  struct SyntaxBinary : SyntaxBase
  {
    std::string op;
    std::unique_ptr<SyntaxBase> lhs, rhs;
    SyntaxBinary(unsigned int p,
                 std::string & o,
                 std::unique_ptr<SyntaxBase> l,
                 std::unique_ptr<SyntaxBase> r)
      : SyntaxBase(p), op(o), lhs(std::move(l)), rhs(std::move(r))
    {
    }
  };

  struct SyntaxCall : SyntaxBase
  {
    std::unique_ptr<SyntaxBase> callee;
    std::vector<std::unique_ptr<SyntaxBase>> args;
    SyntaxCall(unsigned int p,
               std::unique_ptr<SyntaxBase> c,
               std::vector<std::unique_ptr<SyntaxBase>> a)
      : SyntaxBase(p), callee(std::move(c)), args(std::move(a))
    {
    }
  };
  ///@}

  /**
   *
   */
  std::unique_ptr<SyntaxBase> parseExpression(unsigned int min_binding_power = 0);

  /**
   * Check if parsing was properly completed
   */
  void verify();

private:
  /**
   * Function expression lexer
   */
  Lexer _lexer;
  /**
   * Current token
   */
  Lexer::Token _token;
  /**
   * Supported prefix operators
   */
  static inline const std::unordered_set<std::string> _prefix_operators = {"-", "!"};
  /**
   * Supported infix operators
   */
  static inline const std::unordered_set<std::string> _infix_operators = {
      "+", "-", "*", "/", "%", "^", "<", "<=", ">", ">=", "=", "!=", "&", "|"};

  /**
   * Get the next token
   */
  void advance() { _token = _lexer.next(); }

  /**
   *
   */
  std::unique_ptr<SyntaxBase> parsePrimary();

  /**
   * Print a pretty error showing the position of error
   * @param message The error message
   */
  [[noreturn]] void parserError(const std::string & message);

  /**
   * Get the binding power and right-associativity of an infix operator
   * @param op The operator string
   * @returns The binding power and right-associativity of the operator
   */
  std::pair<unsigned int, bool> infixInfo(const std::string & op)
  {
    if (op == "^")
      return {60, true};
    if (op == "*" || op == "/" || op == "%")
      return {50, false};
    if (op == "+" || op == "-")
      return {40, false};
    if (op == "<" || op == "<=" || op == ">" || op == ">=")
      return {35, false};
    if (op == "=" || op == "!=")
      return {30, false};
    if (op == "&")
      return {20, false};
    if (op == "|")
      return {10, false};

    parserError("unsupported infix operator '" + op + "'.");
  }

  /**
   * Prefix operator binding power (should be higher than infix operators)
   */
  static constexpr unsigned int _prefix_binding_power = 70;
};

/**
 * Reverse Polish Notation (RPN) builder
 */
class RPNBuilder
{
public:
  /**
   * Constructor
   * @param expression The function expression
   */
  RPNBuilder(const std::string & expression);

  /**
   * RPN opcode
   */
  enum class Opcode
  {
    NUM,
    VAR,
    NEG,
    // Binary operators
    ADD,
    SUB,
    MUL,
    DIV,
    AND,
    OR,
    EQ,
    NEQ,
    LT,
    LEQ,
    GT,
    GEQ,
    // Functions
    ABS,
    ACOS,
    ACOSH,
    ASIN,
    ASINH,
    ATAN,
    ATAN2,
    ATANH,
    CBRT,
    CEIL,
    COS,
    COSH,
    COT,
    CSC,
    EXP,
    EXP2,
    FLOOR,
    HYPOT,
    IF,
    INT,
    LOG,
    LOG2,
    LOG10,
    MAX,
    MIN,
    POW,
    SEC,
    SIN,
    SINH,
    SQRT,
    TAN,
    TANH,
    TRUNC
  };

  /**
   * RPN instruction
   */
  struct Instruction
  {
    // Opcode
    Opcode op;
    // Original string
    std::string text;
    // Constant of variable index
    unsigned int arg = libMesh::invalid_uint;
  };

  /**
   * Get the head pointer of AST
   */
  auto getAST() { return _ast.get(); }

  /**
   * Build RPN from AST
   */
  void build(const PrattParser::SyntaxBase * syntax);

  /**
   * Print AST for debugging
   */
  void printAST(const ConsoleStream & console,
                const PrattParser::SyntaxBase * syntax,
                unsigned int indent = 0);

  /**
   * Print RPN for debugging
   */
  void printRPN(const ConsoleStream & console);

private:
  /**
   * Map from binary operators to opcodes
   */
  static inline const std::map<std::string, Opcode> _binary_opcode_map = {{"+", Opcode::ADD},
                                                                          {"-", Opcode::SUB},
                                                                          {"*", Opcode::MUL},
                                                                          {"/", Opcode::DIV},
                                                                          {"^", Opcode::POW},
                                                                          {"&", Opcode::AND},
                                                                          {"|", Opcode::OR},
                                                                          {"=", Opcode::EQ},
                                                                          {"!=", Opcode::NEQ},
                                                                          {"<", Opcode::LT},
                                                                          {"<=", Opcode::LEQ},
                                                                          {">", Opcode::GT},
                                                                          {">=", Opcode::GEQ}};

  /**
   * Map from functions to opcodes and the expected number of arguments
   */
  static inline const std::map<std::string, std::pair<Opcode, unsigned int>> _function_opcode_map =
      {{"abs", {Opcode::ABS, 1}},     {"acos", {Opcode::ACOS, 1}},   {"acosh", {Opcode::ACOSH, 1}},
       {"asin", {Opcode::ASIN, 1}},   {"asinh", {Opcode::ASINH, 1}}, {"atan", {Opcode::ATAN, 1}},
       {"atan2", {Opcode::ATAN2, 2}}, {"atanh", {Opcode::ATANH, 1}}, {"cbrt", {Opcode::CBRT, 1}},
       {"ceil", {Opcode::CEIL, 1}},   {"cos", {Opcode::COS, 1}},     {"cosh", {Opcode::COSH, 1}},
       {"cot", {Opcode::COT, 1}},     {"csc", {Opcode::CSC, 1}},     {"exp", {Opcode::EXP, 1}},
       {"exp2", {Opcode::EXP2, 1}},   {"floor", {Opcode::FLOOR, 1}}, {"hypot", {Opcode::HYPOT, 2}},
       {"if", {Opcode::IF, 3}},       {"int", {Opcode::INT, 1}},     {"log", {Opcode::LOG, 1}},
       {"log2", {Opcode::LOG2, 1}},   {"log10", {Opcode::LOG10, 1}}, {"max", {Opcode::MAX, 2}},
       {"min", {Opcode::MIN, 2}},     {"pow", {Opcode::POW, 2}},     {"sec", {Opcode::SEC, 1}},
       {"sin", {Opcode::SIN, 1}},     {"sinh", {Opcode::SINH, 1}},   {"sqrt", {Opcode::SQRT, 1}},
       {"tan", {Opcode::TAN, 1}},     {"tanh", {Opcode::TANH, 1}},   {"trunc", {Opcode::TRUNC, 1}}};

  /**
   * Pratt parser
   */
  PrattParser _parser;
  /**
   * AST built by parser
   */
  std::unique_ptr<PrattParser::SyntaxBase> _ast;
  /**
   * RPN sequence
   */
  std::vector<Instruction> _rpn;
  /**
   * Numbers used in the function
   */
  std::vector<Real> _numbers;
  /**
   * Variables used in the function
   */
  std::map<std::string, unsigned int> _variables;

  /**
   * Add a parsed function constant
   * @param number The constant
   * @returns The constant index
   */
  unsigned int addNumber(Real number);

  /**
   * Add a parsed function variable
   * @param name The variable name
   * @returns The variable index
   */
  unsigned int addVariable(const std::string & name);

  /**
   * Print a pretty error showing the position of error
   * @param syntax The erroneous syntax
   * @param message The error message
   */
  [[noreturn]] void builderError(const PrattParser::SyntaxBase * syntax,
                                 const std::string & message);
};

} // namespace Moose::Kokkos
