//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctionParser.h"

#include "MooseUtils.h"

namespace Moose::Kokkos
{

PEGParser::PEGParser(const std::string & expression, const ConsoleStream * console)
  : _parser(R"(# Entry
               Start           <- _ Expr _

               # Whitespace helpers (ignored: never appear as AST nodes)
               ~WS             <- [ \t\r\n]*
               ~_              <- WS

               # Expressions (lowest precedence at the top)
               Expr            <- OrExpr

               # Logical OR (left-assocative via tail recursion)
               OrExpr          <- AndExpr OrTail?
               OrTail          <- _ OrOp _ AndExpr OrTail?
               OrOp            <- '|'

               # Logical AND (left-assocative via tail recursion)
               AndExpr         <- CmpExpr AndTail?
               AndTail         <- _ AndOp _ CmpExpr AndTail?
               AndOp           <- '&'

               # Comparisons (binary, non-associative: at most one comparison)
               CmpExpr         <- AddExpr (_ CompOp _ AddExpr)?
               CompOp          <- '<=' / '>=' / '<' / '>' / '=' / '!='

               # Addition/subtraction (left-assocative via tail recursion)
               AddExpr         <- MulExpr AddTail?
               AddTail         <- _ AddOp _ MulExpr AddTail?
               AddOp           <- '+' / '-'

               # Multiplication/division (left-assocative via tail recursion)
               MulExpr         <- UnaryExpr MulTail?
               MulTail         <- _ MulOp _ UnaryExpr MulTail?
               MulOp           <- '*' / '/'

               # Unary prefix (strictly unary; right-recursive chain)
               UnaryExpr       <- (_ UnaryOp _ UnaryExpr) / PowExpr
               UnaryOp         <- '-' / '!'

               # Exponentiation (binary, right-associative)
               PowExpr         <- Primary (_ PowOp _ PowExpr)?
               PowOp           <- '^'

               # Atoms
               Primary         <- FunctionCall
                                / Number
                                / Identifier
                                / '(' _ Expr _ ')'

               # Function calls with zero or more arguments
               FunctionCall    <- Identifier _ '(' _ ArgList? _ ')' { no_ast_opt }
               ArgList         <- Expr (_ ',' _ Expr)*

               # Identifiers
               Identifier      <- [a-zA-Z_] [a-zA-Z0-9_]*

               # Numbers (integer/real/scientific)
               Number          <- (# .5 or .5e-2
                                   '.' [0-9]+ ([eE] [+-]? [0-9]+)?
                                   /
                                   # 2. or 2.0 or 42 or with exponent
                                   [0-9]+ ('.' [0-9]*)? ([eE] [+-]? [0-9]+)?)
            )"),
    _expression(expression)
{
  _parser.enable_ast();

  if (console)
    _parser.set_logger(
        [&](std::size_t line, std::size_t col, const std::string & msg, const std::string &)
        { *console << line << ":" << col << ": " << msg << std::endl; });

  if (!_parser.parse(expression.c_str(), _ast))
    mooseError("Kokkos parsed function error: expression parsing failed.");

  _ast = _parser.optimize_ast(_ast);
}

void
RPNBuilder::build(const peg::Ast & ast)
{
  checkFinalized();

  if (ast.name == "Start" || ast.name == "Primary" || ast.name == "ArgList")
    for (const auto & node : ast.nodes)
      build(*node);
  else if (ast.name == "Number")
    _rpn.push_back({Opcode::NUM, ast.token_to_string(), addNumber(ast.token_to_number<Real>())});
  else if (ast.name == "Identifier")
  {
    if (_function_opcode_map.count(ast.token_to_string()))
      builderError(ast, "variable name '" + ast.token_to_string() + "' is reserved for function.");

    _rpn.push_back({Opcode::VAR, ast.token_to_string(), addVariable(ast.token_to_string())});
  }
  else if (ast.name == "UnaryExpr")
  {
    // nodes: [UnaryOp, UnaryExpr]
    build(*ast.nodes[1]);
    const auto op = ast.nodes[0]->token_to_string();
    _rpn.push_back({libmesh_map_find(_unary_opcode_map, op), op});
  }
  else if (ast.name == "OrExpr" || ast.name == "AndExpr" || ast.name == "AddExpr" ||
           ast.name == "MulExpr")
  {
    // nodes: [LeftOperand, Tail]
    build(*ast.nodes[0]);
    build(*ast.nodes[1]);
  }
  else if (ast.name == "OrTail" || ast.name == "AndTail" || ast.name == "AddTail" ||
           ast.name == "MulTail")
  {
    // nodes: [Op, RightOperand, ?NextTail]
    build(*ast.nodes[1]);
    const auto op = ast.nodes[0]->token_to_string();
    _rpn.push_back({libmesh_map_find(_binary_opcode_map, op), op});
    if (ast.nodes.size() == 3)
      build(*ast.nodes[2]);
  }
  else if (ast.name == "CmpExpr" || ast.name == "PowExpr")
  {
    // nodes: [LeftOperand, Op, RightOperand]
    build(*ast.nodes[0]);
    build(*ast.nodes[2]);
    const auto op = ast.nodes[1]->token_to_string();
    _rpn.push_back({libmesh_map_find(_binary_opcode_map, op), op});
  }
  else if (ast.name == "FunctionCall")
  {
    // nodes: [Identifier, ?ArgList]
    const auto func = ast.nodes[0]->token_to_string();

    auto it = _function_opcode_map.find(func);
    if (it == _function_opcode_map.end())
      builderError(ast, "unsupported function '" + func + "()'.");

    if (ast.nodes.size() == 1)
      builderError(ast, "argument(s) missing for function '" + func + "()'.");

    const auto & args = *ast.nodes[1];

    unsigned int num_args = 1;
    if (args.name == "ArgList")
      num_args = args.nodes.size();

    if (num_args != it->second.second)
      builderError(ast, func + "() expects " + std::to_string(it->second.second) + " argument(s).");

    build(args);

    _rpn.push_back({it->second.first, func});
  }
}

void
RPNBuilder::printRPN(const ConsoleStream & console) const
{
  std::vector<std::string> instructions;

  for (const auto & instruction : _rpn)
    instructions.push_back(instruction.text);

  console << std::endl << MooseUtils::join(instructions, " ") << std::endl << std::endl;
}

void
RPNBuilder::addDefaultVariables()
{
  checkFinalized();

  _has_default_variables = true;

  addVariable("x");
  addVariable("y");
  addVariable("z");
  addVariable("t");
}

unsigned int
RPNBuilder::addNumber(Real number)
{
  checkFinalized();

  _numbers.push_back(number);
  return _numbers.size() - 1;
}

unsigned int
RPNBuilder::addVariable(const std::string & name)
{
  checkFinalized();

  auto it = _variables.find(name);
  if (it != _variables.end())
    return it->second.idx;

  // Default variables: x, y, z, t
  unsigned int idx = _variables.size() + (_has_default_variables ? 0 : 4);

  _variables[name].idx = idx;

  return idx;
}

void
RPNBuilder::associateScalar(const std::string & name, const Real * scalar)
{
  checkFinalized();

  auto it = _variables.find(name);
  if (it == _variables.end())
    mooseError("Kokkos parsed function error: variable '",
               name,
               "' was not added but attempted to associate a value.");

  if (it->second.associated())
    mooseError("Kokkos parsed function error: variable '",
               name,
               "' is already associated and cannot associate another value.");

  it->second.scalar = scalar;
}

void
RPNBuilder::associateProperty(const std::string & name, const MaterialProperty<Real> * property)
{
  checkFinalized();

  auto it = _variables.find(name);
  if (it == _variables.end())
    mooseError("Kokkos parsed function error: variable '",
               name,
               "' was not added but attempted to associate a material property.");

  if (it->second.associated())
    mooseError("Kokkos parsed function error: variable '",
               name,
               "' is already associated and cannot associate another material property.");

  it->second.property = property;
}

void
RPNBuilder::associateField(const std::string & name, const VariableValue * field)
{
  checkFinalized();

  auto it = _variables.find(name);
  if (it == _variables.end())
    mooseError("Kokkos parsed function error: variable '",
               name,
               "' was not added but attempted to associate a field variable.");

  if (it->second.associated())
    mooseError("Kokkos parsed function error: variable '",
               name,
               "' is already associated and cannot associate another field variable.");

  it->second.field = field;
}

void
RPNBuilder::associateFunction(const std::string & name, const Function * function)
{
  checkFinalized();

  auto it = _variables.find(name);
  if (it == _variables.end())
    mooseError("Kokkos parsed function error: variable '",
               name,
               "' was not added but attempted to associate a function.");

  if (it->second.associated())
    mooseError("Kokkos parsed function error: variable '",
               name,
               "' is already associated and cannot associate another function.");

  it->second.function = function;
}

void
RPNBuilder::builderError(const peg::Ast & ast, const std::string & message) const
{
  std::string arrow(ast.column, ' ');
  arrow.back() = '^';

  mooseError("Kokkos parsed function error: ", message, "\n", _parser.expression(), "\n", arrow);
}

void
RPNBuilder::checkFinalized()
{
  if (_finalized)
    mooseError("Kokkos parsed function error: cannot update builder after finalization.");
}

RPNEvaluator::RPNEvaluator(const RPNEvaluator & evaluator)
  : _rpn(evaluator._rpn),
    _numbers(evaluator._numbers),
    _variables(evaluator._variables),
    _num_scalars(evaluator._num_scalars),
    _num_fields(evaluator._num_fields),
    _num_properties(evaluator._num_properties),
    _num_functions(evaluator._num_functions)
{
  _scalars.create(_num_scalars);
  _fields.create(_num_fields);
  _properties.create<false>(_num_properties);
  _functions.create<false>(_num_functions);

  for (const auto i : make_range(_variables.size()))
  {
    auto ptr = evaluator._pointers[i];
    if (!ptr)
      continue;

    const auto type = _variables[i].first;
    const auto idx = _variables[i].second;

    if (type == VariableType::SCALAR)
      _scalars[idx] = *static_cast<const Real *>(ptr);
    else if (type == VariableType::FIELD)
      _fields[idx] = *static_cast<const VariableValue *>(ptr);
    else if (type == VariableType::MATERIAL)
      _properties.emplace({idx}, *static_cast<const MaterialProperty<Real> *>(ptr));
    else if (type == VariableType::FUNCTION)
      _functions.emplace({idx}, *static_cast<const Function *>(ptr));
  }

  _scalars.copyToDevice();
  _fields.copyToDevice();
  _properties.copyToDevice();
  _functions.copyToDevice();
}

void
RPNEvaluator::init(const RPNBuilder & builder)
{
  if (!builder.finalized())
    mooseError("Kokkos parsed function error: attempted to initialize evaluator with a builder "
               "which was not finalized.");

  const auto & numbers = builder.getNumbers();
  const auto & variables = builder.getVariables();
  const auto & rpn = builder.getRPN();

  _numbers = numbers;

  _variables.create(variables.size() + (builder.hasDefaultVariables() ? 0 : 4));
  _pointers.resize(_variables.size(), nullptr);

  _num_scalars = 0;
  _num_fields = 0;
  _num_properties = 0;
  _num_functions = 0;

  for (const auto & [name, var] : variables)
  {
    if (var.scalar)
    {
      _variables[var.idx].first = VariableType::SCALAR;
      _variables[var.idx].second = _num_scalars++;
      _pointers[var.idx] = var.scalar;
    }
    else if (var.field)
    {
      _variables[var.idx].first = VariableType::FIELD;
      _variables[var.idx].second = _num_fields++;
      _pointers[var.idx] = var.field;
    }
    else if (var.property)
    {
      _variables[var.idx].first = VariableType::MATERIAL;
      _variables[var.idx].second = _num_properties++;
      _pointers[var.idx] = var.property;
    }
    else if (var.function)
    {
      _variables[var.idx].first = VariableType::FUNCTION;
      _variables[var.idx].second = _num_functions++;
      _pointers[var.idx] = var.function;
    }
    else if (var.idx >= 4)
      mooseError(
          "Kokkos parsed function error: variable '", name, "' was not associated with any value.");
  }

  _variables.copyToDevice();

  _rpn.create(rpn.size());

  for (const auto i : make_range(rpn.size()))
    _rpn[i] = {rpn[i].op, rpn[i].arg};

  _rpn.copyToDevice();
}

} // namespace Moose::Kokkos
