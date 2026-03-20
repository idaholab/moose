#include "hit/braceexpr.h"

#include <cstdlib>
#include <iostream>

namespace hit
{

namespace
{

class BraceTemplateParser
{
public:
  BraceTemplateParser(const std::string & input, BraceParseError * error)
    : _input(input), _error(error)
  {
  }

  BraceTemplate parse()
  {
    auto result = parseTemplate(ParseMode::TopLevel);
    if (!eof())
      failHere("unexpected trailing input");
    return result;
  }

private:
  enum class ParseMode
  {
    TopLevel,
    Argument
  };

  struct Position
  {
    std::size_t index;
    std::size_t line;
    std::size_t column;
  };

  bool eof() const { return _pos >= _input.size(); }

  char current() const { return eof() ? '\0' : _input[_pos]; }

  bool startsExpr() const { return _input.compare(_pos, 2, "${") == 0; }

  static bool isWhitespace(char c)
  {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
  }

  Position mark() const { return {_pos, _line, _column}; }

  void advance()
  {
    if (eof())
      return;

    if (_input[_pos] == '\n')
    {
      ++_line;
      _column = 1;
    }
    else
      ++_column;

    ++_pos;
  }

  void advance(std::size_t count)
  {
    for (std::size_t i = 0; i < count; ++i)
      advance();
  }

  void skipWhitespace()
  {
    while (!eof() && isWhitespace(current()))
      advance();
  }

  [[noreturn]] void fail(const Position & pos, const std::string & message) const
  {
    if (_error && _error->message.empty())
    {
      _error->line = pos.line;
      _error->column = pos.column;
      _error->message = message;
    }
    throw Error("invalid brace expression");
  }

  [[noreturn]] void failHere(const std::string & message) const { fail(mark(), message); }

  std::string slice(const Position & begin, const Position & end) const
  {
    return _input.substr(begin.index, end.index - begin.index);
  }

  BraceTemplate parseTemplate(ParseMode mode)
  {
    BraceTemplate root;
    while (!eof())
    {
      if (startsExpr())
        root.parts().push_back(parseExpression());
      else if (mode == ParseMode::Argument && (current() == '}' || isWhitespace(current())))
        break;
      else
      {
        const auto start = mark();
        while (!eof() && !startsExpr() &&
               !(mode == ParseMode::Argument && (current() == '}' || isWhitespace(current()))))
          advance();
        root.parts().push_back(slice(start, mark()));
      }
    }

    return root;
  }

  std::shared_ptr<BraceExpr> parseExpression()
  {
    if (!startsExpr())
      failHere("missing opening '${' in brace expression");

    const auto start = mark();
    auto expr = std::make_shared<BraceExpr>();

    advance(2);
    skipWhitespace();

    if (eof())
      failHere("missing closing '}' in brace expression");

    while (!eof() && current() != '}')
    {
      expr->args().push_back(parseArgument());
      skipWhitespace();
    }

    if (eof() || current() != '}')
      failHere("missing closing '}' in brace expression");

    advance();

    const auto end = mark();
    expr->line() = start.line;
    expr->column() = start.column;
    expr->position() = start.index;
    expr->length() = end.index - start.index;
    expr->text() = slice(start, end);
    return expr;
  }

  BraceArg parseArgument()
  {
    BraceArg arg;
    auto & parts = arg.value().parts();

    while (!eof() && current() != '}' && !isWhitespace(current()))
      if (startsExpr())
        parts.push_back(parseExpression());
      else
      {
        const auto start = mark();
        while (!eof() && !startsExpr() && current() != '}' && !isWhitespace(current()))
          advance();
        parts.push_back(slice(start, mark()));
      }

    if (parts.empty())
      failHere("expected brace-expression argument");

    return arg;
  }

  const std::string & _input;
  BraceParseError * _error = nullptr;
  std::size_t _pos = 0;
  std::size_t _line = 1;
  std::size_t _column = 1;
};

std::pair<int, int>
fieldValueStart(const Field * n)
{
  int column = n->column() + static_cast<int>(n->path().size()) + 3;
  if (quoteChar(n->val()) != "")
    ++column;
  return {n->line(), column};
}

std::pair<int, int>
advanceLocation(int line, int column, const std::string & text)
{
  if (text.empty())
    return {line, column};

  for (std::size_t i = 1; i < text.size(); ++i)
    if (text[i] == '\n')
    {
      ++line;
      column = 1;
    }
    else
      ++column;

  return {line, column};
}

ErrorMessage
fieldValueErrorMessage(Field * n,
                       std::size_t rel_line,
                       std::size_t rel_column,
                       const std::string & span_text,
                       const std::string & message)
{
  const auto [base_line, base_column] = fieldValueStart(n);

  int start_line = base_line + static_cast<int>(rel_line) - 1;
  int start_column =
      rel_line == 1 ? base_column + static_cast<int>(rel_column) - 1 : static_cast<int>(rel_column);

  const auto [end_line, end_column] = advanceLocation(start_line, start_column, span_text);
  return ErrorMessage(message, n->filename(), start_line, start_column, end_line, end_column);
}

} // namespace

std::string
BraceNode::str(int indent)
{
  std::string indstr;
  for (int i = 0; i < indent; i++)
    indstr += "    ";

  if (_val != "")
    return indstr + _val + "\n";

  std::string s = indstr + "${\n";
  for (auto & n : _list)
  {
    s += n.str(indent + 1);
  }
  s += indstr + "}\n";
  return s;
}

BraceNode &
BraceNode::append()
{
  _list.push_back(BraceNode());
  return _list.back();
}

bool
BraceTemplate::pureExpression() const
{
  return _parts.size() == 1 && std::holds_alternative<std::shared_ptr<BraceExpr>>(_parts.front());
}

bool
BraceTemplate::hasLiteralText() const
{
  for (const auto & part : _parts)
    if (std::holds_alternative<std::string>(part) && !std::get<std::string>(part).empty())
      return true;

  return false;
}

std::size_t
BraceTemplate::expressionCount() const
{
  std::size_t count = 0;
  for (const auto & part : _parts)
    if (std::holds_alternative<std::shared_ptr<BraceExpr>>(part))
      ++count;

  return count;
}

BraceTemplate
parseBraceTemplate(const std::string & input, BraceParseError * error)
{
  return BraceTemplateParser(input, error).parse();
}

EvalResult
EnvEvaler::eval(Field * /*n*/, const std::vector<std::string> & args, BraceExpander & /*exp*/)
{
  EvalResult result;
  result.kind = Field::Kind::String;
  std::string var = args.front();
  auto data = std::getenv(var.c_str());
  if (data)
    result.value = data;
  return result;
}

EvalResult
RawEvaler::eval(Field * /*n*/, const std::vector<std::string> & args, BraceExpander & /*exp*/)
{
  EvalResult result;
  result.kind = Field::Kind::String;
  for (auto & arg : args)
    result.value += arg;
  return result;
}

EvalResult
ReplaceEvaler::eval(Field * n, const std::vector<std::string> & args, BraceExpander & exp)
{
  EvalResult result;
  auto & var = args.front();
  std::string used_path;
  if (auto src = exp.findReferencedField(n, var, &used_path))
  {
    const auto & resolved = exp.resolve(src);
    result.used.push_back(used_path);
    result.kind = resolved.kind != Field::Kind::None ? resolved.kind : src->kind();
    result.value = resolved.value;
    return result;
  }

  exp.errors.push_back(exp.currentExpressionErrorMessage(
      n, "no variable '" + var + "' found for substitution expression in '" + n->fullpath() + "'"));
  result.value = n->val();
  return result;
}

void
BraceExpander::registerEvaler(const std::string & name, Evaler & ev)
{
  _evalers[name] = &ev;
}

void
BraceExpander::walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, Node * n)
{
  auto f = dynamic_cast<Field *>(n);
  if (!f)
    throw Error("BraceExpander cannot walk non-Field-type nodes", n);

  try
  {
    const auto & result = resolve(f);
    used.insert(used.end(), result.used.begin(), result.used.end());
  }
  catch (Error & err)
  {
    errors.insert(errors.end(), err.error_messages.begin(), err.error_messages.end());
    return;
  }
}

std::string
BraceExpander::expand(Field * f, const std::string & input)
{
  BraceTemplate root;
  BraceParseError error;
  try
  {
    root = parseBraceTemplate(input, &error);
  }
  catch (const Error &)
  {
    throw syntaxError(f, error);
  }

  auto result = evaluate(f, root);
  return result.value;
}

Field *
BraceExpander::findReferencedField(Field * n, const std::string & path, std::string * used_path)
{
  Node * curr = n;
  while ((curr = curr->parent()))
  {
    auto src = curr->find(path);
    if (src && src != n && src->type() == NodeType::Field)
    {
      if (used_path)
        *used_path = pathJoin({curr->fullpath(), path});
      return dynamic_cast<Field *>(src);
    }
  }

  return nullptr;
}

const EvalResult &
BraceExpander::resolve(Field * n)
{
  auto & entry = _resolved[n];
  if (entry.state == ResolveState::Resolved)
    return entry.result;

  if (entry.state == ResolveState::Resolving)
  {
    std::string chain;
    for (const auto * field : _resolving_stack)
    {
      if (!chain.empty())
        chain += " -> ";
      chain += field->fullpath();
    }
    if (!chain.empty())
      chain += " -> ";
    chain += n->fullpath();
    throw Error("cyclic brace-expression dependency detected: " + chain, n);
  }

  entry.state = ResolveState::Resolving;
  _resolving_stack.push_back(n);

  auto restore = [&]()
  {
    _resolving_stack.pop_back();
    if (entry.state != ResolveState::Resolved)
      entry.state = ResolveState::Unvisited;
  };

  try
  {
    auto unquoted = n->val();
    const auto quote = quoteChar(n->val());
    if (quote != "")
      unquoted = unquoted.substr(1, unquoted.size() - 2);

    entry.result.value = unquoted;
    entry.result.kind = n->kind();

    if (unquoted.find("${") != std::string::npos)
    {
      BraceTemplate root;
      BraceParseError error;
      const auto error_count = errors.size();
      try
      {
        root = parseBraceTemplate(unquoted, &error);
      }
      catch (const Error &)
      {
        throw syntaxError(n, error);
      }

      entry.result = evaluate(n, root);
      if (errors.size() != error_count)
      {
        // Preserve the original field text when brace expansion recorded an error.
        entry.result.value = unquoted;
        entry.result.kind = n->kind();
      }
      else if (!root.pureExpression() && (root.expressionCount() > 0 || root.hasLiteralText()))
        entry.result.kind = Field::Kind::String;
      else if (root.pureExpression() && entry.result.kind == Field::Kind::None)
        entry.result.kind = n->kind();
    }

    std::string rendered = entry.result.value;
    if (entry.result.kind == Field::Kind::String && quote != "")
      rendered = quote + rendered + quote;

    n->setVal(rendered, entry.result.kind);
    entry.state = ResolveState::Resolved;
    restore();
    return entry.result;
  }
  catch (...)
  {
    restore();
    throw;
  }
}

EvalResult
BraceExpander::evaluate(Field * n, const BraceTemplate & expr)
{
  EvalResult result;
  for (const auto & part : expr.parts())
    if (std::holds_alternative<std::string>(part))
    {
      result.value += std::get<std::string>(part);
      result.kind = Field::Kind::String;
    }
    else
    {
      auto child = evaluate(n, *std::get<std::shared_ptr<BraceExpr>>(part));
      result.value += child.value;
      result.used.insert(result.used.end(), child.used.begin(), child.used.end());
      result.kind = expr.pureExpression() ? child.kind : Field::Kind::String;
    }

  return result;
}

EvalResult
BraceExpander::evaluate(Field * n, const BraceExpr & expr)
{
  std::vector<std::string> expanded_args;
  std::vector<std::vector<std::string>> child_used;
  expanded_args.reserve(expr.args().size());
  child_used.reserve(expr.args().size());
  for (const auto & arg : expr.args())
  {
    auto child = evaluate(n, arg.value());
    expanded_args.push_back(child.value);
    child_used.push_back(std::move(child.used));
  }

  if (expanded_args.empty())
    throw expressionError(n, expr, "empty brace expression in '" + n->fullpath() + "'");

  EvalResult result;
  const auto * previous_expr = _active_expr;
  _active_expr = &expr;

  // Just use replace expander if one arg given
  try
  {
    if (expanded_args.size() == 1)
      result = _replace.eval(n, expanded_args, *this);
    else
    {
      auto cmd = expanded_args.front();
      if (_evalers.count(cmd) == 0)
        throw expressionError(
            n, expr, "invalid brace-expression command '" + cmd + "' in '" + n->fullpath() + "'");
      expanded_args.erase(expanded_args.begin());
      child_used.erase(child_used.begin());
      result = _evalers[cmd]->eval(n, expanded_args, *this);
    }
  }
  catch (...)
  {
    _active_expr = previous_expr;
    throw;
  }
  _active_expr = previous_expr;

  for (const auto & used_paths : child_used)
    result.used.insert(result.used.end(), used_paths.begin(), used_paths.end());

  return result;
}

Error
BraceExpander::syntaxError(Field * n, const BraceParseError & error) const
{
  const auto message = "invalid brace-expression syntax in '" + n->fullpath() + "'" +
                       (error.message.empty() ? "" : ": " + error.message);

  if (!error.message.empty())
    return Error(std::vector<ErrorMessage>{
        fieldValueErrorMessage(n, error.line, error.column, "${", message)});

  return Error(message, n);
}

Error
BraceExpander::expressionError(Field * n, const BraceExpr & expr, const std::string & message) const
{
  return Error(std::vector<ErrorMessage>{expressionErrorMessage(n, expr, message)});
}

ErrorMessage
BraceExpander::expressionErrorMessage(Field * n,
                                      const BraceExpr & expr,
                                      const std::string & message) const
{
  const auto & span_text = expr.text().empty() ? std::string("${") : expr.text();
  return fieldValueErrorMessage(n, expr.line(), expr.column(), span_text, message);
}

ErrorMessage
BraceExpander::currentExpressionErrorMessage(Field * n, const std::string & message) const
{
  if (_active_expr)
    return expressionErrorMessage(n, *_active_expr, message);

  return ErrorMessage(message, n);
}

Error
BraceExpander::currentExpressionError(Field * n, const std::string & message) const
{
  return Error(std::vector<ErrorMessage>{currentExpressionErrorMessage(n, message)});
}

size_t
parseBraceNode(const std::string & input, size_t start, BraceNode & n)
{
  n.offset() = start;
  n.len() = 0;
  n.val().clear();
  n.list().clear();

  if (input.find("${", start) != start)
    throw Error("missing opening '${' in brace expression");

  auto pos = start + 2;
  while (pos < input.size())
  {
    if (input.find("${", pos) == pos)
      pos = parseBraceNode(input, pos, n.append());
    else if (input[pos] == '}')
    {
      ++pos;
      n.len() = pos - n.offset();
      return pos;
    }
    else
      ++pos;
  }

  throw Error("missing closing '}' in brace expression");
}

} // namespace hit
