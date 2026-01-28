//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"

#include "ConsoleStreamInterface.h"

#include "peglib.h"

namespace Moose::Kokkos
{

class FunctionBase;

/**
 * Parsing Expression Grammar (PEG)
 */
class PEGParser
{
public:
  /**
   * Constructor
   * @param expression The function expression
   * @param console The console object
   */
  PEGParser(const std::string & expression, const ConsoleStream * console = nullptr);

  /**
   * Get AST
   * @returns The AST
   */
  auto ast() const { return _ast; }
  /**
   * Get input expression
   * @returns The input expression
   */
  const auto & expression() const { return _expression; }

private:
  /**
   * Parser object
   */
  peg::parser _parser;
  /**
   * Abstract Syntax Tree (AST)
   */
  std::shared_ptr<peg::Ast> _ast;
  /**
   * Input expression
   */
  const std::string & _expression;
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
   * @param console The console object
   */
  RPNBuilder(const std::string & expression, const ConsoleStream * console = nullptr)
    : _parser(expression, console)
  {
  }

  /**
   * RPN opcode
   */
  enum class Opcode
  {
    NUM,
    VAR,
    NEG,
    NOT,
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
   * Variable identifier
   */
  struct Variable
  {
    // Variable index
    unsigned int idx;
    // Pointer to variable value
    const Real * value = nullptr;
  };

  /**
   * Build RPN from AST
   * @param ast The current node
   */
  ///@{
  void build(const peg::Ast & ast);
  void build() { build(*_parser.ast()); }
  ///@}

  /**
   * Print RPN sequence for debugging
   * @param console The console object
   */
  void printRPN(const ConsoleStream & console);
  /**
   * Get RPN sequence
   * @returns The RPN sequence
   */
  const auto & getRPN() const { return _rpn; }

  /**
   * Add default variables
   */
  void addDefaultVariables();
  /**
   * Get whether default variables were added
   * @returns Whether default variables were added
   */
  bool hasDefaultVariables() const { return _has_default_variables; }

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
   * Associate a variable with a value
   * @param name The variable name
   * @param value The pointer to the variable value
   */
  void associateValue(const std::string & name, const Real * value);

  /**
   * Get numbers used in the expression
   * @returns The numbers
   */
  const auto & getNumbers() const { return _numbers; }
  /**
   * Get variables used in the expression
   * @returns The variables
   */
  const auto & getVariables() const { return _variables; }

private:
  /**
   * Map from unary operators to opcodes
   */
  static inline const std::map<std::string, Opcode> _unary_opcode_map = {{"-", Opcode::NEG},
                                                                         {"!", Opcode::NOT}};

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
   * PEG parser
   */
  PEGParser _parser;
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
  std::unordered_map<std::string, Variable> _variables;
  /**
   * Whether default variables were added
   */
  bool _has_default_variables = false;

  /**
   * Find node excluding whitespaces
   * @param ast The root node
   * @param i The node index to find
   * @return The node
   */
  const peg::Ast & getNode(const peg::Ast & ast, const unsigned int i);

  /**
   * Count the number of nodes excluding whitespaces
   * @param ast The root node
   * @return The number of nodes
   */
  unsigned int getNumNodes(const peg::Ast & ast);

  /**
   * Print a pretty error showing the position of error
   * @param ast The erroneous AST node
   * @param message The error message
   */
  [[noreturn]] void builderError(const peg::Ast & ast, const std::string & message);
};

/**
 * Reverse Polish Notation (RPN) evaluator
 */
class RPNEvaluator
{
  using Opcode = RPNBuilder::Opcode;

public:
  /**
   * Constructor
   * @param builder The RPN builder
   */
  RPNEvaluator(const RPNBuilder & builder) : _builder(builder) {}
  /**
   * Copy constructor for parallel dispatch
   */
  RPNEvaluator(const RPNEvaluator & evaluator);

  /**
   * Initialize RPN evaluator
   */
  void init();

  /**
   * Evaluate RPN at point (t,x,y,z)
   * @param t The time
   * @param p The location in space (x,y,z)
   * @returns The evaluated value
   */
  KOKKOS_FUNCTION Real eval(const Real t, const Real3 p) const;

private:
  /**
   * RPN instruction
   */
  struct Instruction
  {
    // Opcode
    Opcode op;
    // Constant of variable index
    unsigned int arg = libMesh::invalid_uint;
  };

  /**
   * RPN builder
   */
  const RPNBuilder & _builder;
  /**
   * RPN sequence
   */
  Array<Instruction> _rpn;
  /**
   * Numbers used in the function
   */
  Array<Real> _numbers;
  /**
   * Variables used in the function
   */
  Array<Real> _variables;

  /**
   * Fixed stack size
   */
  static constexpr unsigned int _stack_size = 10;
  /**
   * Epsilon for equality comparison
   */
  static constexpr double _epsilon = 1.0e-12;
};

#define KOKKOS_FPARSER_COMPARE(code, op, epsilon)                                                  \
  case Opcode::code:                                                                               \
    stack[head - 2] = stack[head - 2] - stack[head - 1] op epsilon;                                \
    --head;                                                                                        \
    break

#define KOKKOS_FPARSER_BINARY(code, op)                                                            \
  case Opcode::code:                                                                               \
    stack[head - 2] = stack[head - 2] op stack[head - 1];                                          \
    --head;                                                                                        \
    break

#define KOKKOS_FPARSER_FUNCTION_1(code, func)                                                      \
  case Opcode::code:                                                                               \
    stack[head - 1] = ::Kokkos::func(stack[head - 1]);                                             \
    break

#define KOKKOS_FPARSER_FUNCTION_2(code, func)                                                      \
  case Opcode::code:                                                                               \
    stack[head - 2] = ::Kokkos::func(stack[head - 2], stack[head - 1]);                            \
    --head;                                                                                        \
    break

#define KOKKOS_FPARSER_FUNCTION_INV_1(code, func)                                                  \
  case Opcode::code:                                                                               \
    stack[head - 1] = 1.0 / ::Kokkos::func(stack[head - 1]);                                       \
    break

KOKKOS_FUNCTION inline Real
RPNEvaluator::eval(const Real t, const Real3 p) const
{
  Real stack[_stack_size];

  // Stack head position
  unsigned int head = 0;

  for (unsigned int pos = 0; pos < _rpn.size(); ++pos)
  {
    KOKKOS_IF_ON_HOST(if (head == _stack_size)
                          mooseError("Kokkos parsed function error: pre-allocated stack size (",
                                     _stack_size,
                                     ") is insufficient.");)

    const auto inst = _rpn[pos];

    switch (_rpn[pos].op)
    {
      case Opcode::NUM:
        stack[head] = _numbers[inst.arg];
        ++head;
        break;
      case Opcode::VAR:
        if (inst.arg < 3)
          stack[head] = p(inst.arg);
        else if (inst.arg == 3)
          stack[head] = t;
        else
          stack[head] = _variables[inst.arg];
        ++head;
        break;
      case Opcode::NEG:
        stack[head - 1] = -stack[head - 1];
        break;
      case Opcode::NOT:
        stack[head - 1] = !::Kokkos::round(stack[head - 1]);
        break;
      case Opcode::EQ:
        stack[head - 2] = ::Kokkos::abs(stack[head - 2] - stack[head - 1]) <= _epsilon;
        --head;
        break;
      case Opcode::NEQ:
        stack[head - 2] = ::Kokkos::abs(stack[head - 2] - stack[head - 1]) > _epsilon;
        --head;
        break;
      case Opcode::AND:
        stack[head - 2] = ::Kokkos::round(stack[head - 2]) && ::Kokkos::round(stack[head - 1]);
        --head;
        break;
      case Opcode::OR:
        stack[head - 2] = ::Kokkos::round(stack[head - 2]) || ::Kokkos::round(stack[head - 1]);
        --head;
        break;
      case Opcode::IF:
        stack[head - 3] = ::Kokkos::round(stack[head - 3]) ? stack[head - 2] : stack[head - 1];
        head -= 2;
        break;
        KOKKOS_FPARSER_COMPARE(LT, <, -_epsilon);
        KOKKOS_FPARSER_COMPARE(LEQ, <=, _epsilon);
        KOKKOS_FPARSER_COMPARE(GT, >, _epsilon);
        KOKKOS_FPARSER_COMPARE(GEQ, >=, -_epsilon);
        KOKKOS_FPARSER_BINARY(ADD, +);
        KOKKOS_FPARSER_BINARY(SUB, -);
        KOKKOS_FPARSER_BINARY(MUL, *);
        KOKKOS_FPARSER_BINARY(DIV, /);
        KOKKOS_FPARSER_FUNCTION_1(ABS, abs);
        KOKKOS_FPARSER_FUNCTION_1(ACOS, acos);
        KOKKOS_FPARSER_FUNCTION_1(ACOSH, acosh);
        KOKKOS_FPARSER_FUNCTION_1(ASIN, asin);
        KOKKOS_FPARSER_FUNCTION_1(ASINH, asinh);
        KOKKOS_FPARSER_FUNCTION_1(ATAN, atan);
        KOKKOS_FPARSER_FUNCTION_2(ATAN2, atan2);
        KOKKOS_FPARSER_FUNCTION_1(ATANH, atanh);
        KOKKOS_FPARSER_FUNCTION_1(CBRT, cbrt);
        KOKKOS_FPARSER_FUNCTION_1(CEIL, ceil);
        KOKKOS_FPARSER_FUNCTION_1(COS, cos);
        KOKKOS_FPARSER_FUNCTION_1(COSH, cosh);
        KOKKOS_FPARSER_FUNCTION_INV_1(COT, tan);
        KOKKOS_FPARSER_FUNCTION_INV_1(CSC, sin);
        KOKKOS_FPARSER_FUNCTION_1(EXP, exp);
        KOKKOS_FPARSER_FUNCTION_1(EXP2, exp2);
        KOKKOS_FPARSER_FUNCTION_1(FLOOR, floor);
        KOKKOS_FPARSER_FUNCTION_2(HYPOT, hypot);
        KOKKOS_FPARSER_FUNCTION_1(INT, round);
        KOKKOS_FPARSER_FUNCTION_1(LOG, log);
        KOKKOS_FPARSER_FUNCTION_1(LOG2, log2);
        KOKKOS_FPARSER_FUNCTION_1(LOG10, log10);
        KOKKOS_FPARSER_FUNCTION_2(MAX, max);
        KOKKOS_FPARSER_FUNCTION_2(MIN, min);
        KOKKOS_FPARSER_FUNCTION_2(POW, pow);
        KOKKOS_FPARSER_FUNCTION_INV_1(SEC, cos);
        KOKKOS_FPARSER_FUNCTION_1(SIN, sin);
        KOKKOS_FPARSER_FUNCTION_1(SINH, sinh);
        KOKKOS_FPARSER_FUNCTION_1(SQRT, sqrt);
        KOKKOS_FPARSER_FUNCTION_1(TAN, tan);
        KOKKOS_FPARSER_FUNCTION_1(TANH, tanh);
        KOKKOS_FPARSER_FUNCTION_1(TRUNC, trunc);
    }
  }

  KOKKOS_IF_ON_HOST(if (head != 1) mooseError("Kokkos parsed function error: stack was not fully "
                                              "consumed after parsing (",
                                              head - 1,
                                              " remaining). This is likely a parser bug.");)

  return stack[head - 1];
}

} // namespace Moose::Kokkos
