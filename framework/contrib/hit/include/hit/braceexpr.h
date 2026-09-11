#pragma once

#include "hit/parse.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <variant>

namespace hit
{

inline std::string
errormsg(std::string /*fname*/, Node * /*n*/)
{
  return "";
}

template <typename T, typename... Args>
std::string
errormsg(std::string fname, Node * n, T arg, Args... args)
{
  std::stringstream ss;
  if (n && fname.size() > 0)
    ss << fname << ":" << n->line() << "." << n->column() << ": ";
  else if (fname.size() > 0)
    ss << fname << ":0: ";
  ss << arg;
  ss << errormsg("", nullptr, args...);
  return ss.str();
}

inline std::string
errormsg(Node * /*n*/)
{
  return "";
}

template <typename T, typename... Args>
std::string
errormsg(Node * n, T arg, Args... args)
{
  std::stringstream ss;
  if (n)
    ss << n->filename() << ":" << n->line() << "." << n->column() << ": ";
  ss << arg;
  ss << errormsg(nullptr, args...);
  return ss.str();
}

class BraceNode
{
public:
  std::string str(int indent = 0);
  BraceNode & append();

  inline std::vector<BraceNode> & list() { return _list; }
  inline std::string & val() { return _val; }
  inline size_t & offset() { return _offset; }
  inline size_t & len() { return _len; }

private:
  size_t _offset;
  size_t _len;
  std::string _val;
  std::vector<BraceNode> _list;
};

size_t parseBraceNode(const std::string & input, size_t start, BraceNode & n);

class BraceExpr;
using BracePart = std::variant<std::string, std::shared_ptr<BraceExpr>>;

class BraceTemplate
{
public:
  inline std::vector<BracePart> & parts() { return _parts; }
  inline const std::vector<BracePart> & parts() const { return _parts; }
  bool pureExpression() const;
  bool hasLiteralText() const;
  std::size_t expressionCount() const;

private:
  std::vector<BracePart> _parts;
};

class BraceArg
{
public:
  inline BraceTemplate & value() { return _value; }
  inline const BraceTemplate & value() const { return _value; }

private:
  BraceTemplate _value;
};

class BraceExpr
{
public:
  inline std::vector<BraceArg> & args() { return _args; }
  inline const std::vector<BraceArg> & args() const { return _args; }
  inline std::size_t & line() { return _line; }
  inline const std::size_t & line() const { return _line; }
  inline std::size_t & column() { return _column; }
  inline const std::size_t & column() const { return _column; }
  inline std::size_t & position() { return _position; }
  inline const std::size_t & position() const { return _position; }
  inline std::size_t & length() { return _length; }
  inline const std::size_t & length() const { return _length; }
  inline std::string & text() { return _text; }
  inline const std::string & text() const { return _text; }

private:
  std::vector<BraceArg> _args;
  std::size_t _line = 1;
  std::size_t _column = 1;
  std::size_t _position = 0;
  std::size_t _length = 0;
  std::string _text;
};

struct BraceParseError
{
  std::size_t line = 1;
  std::size_t column = 1;
  std::string message;
};

BraceTemplate parseBraceTemplate(const std::string & input, BraceParseError * error = nullptr);
class BraceExpander;

struct EvalResult
{
  std::string value;
  Field::Kind kind = Field::Kind::None;
  std::vector<std::string> used;
};

class Evaler
{
public:
  virtual ~Evaler() {}
  virtual EvalResult
  eval(Field * n, const std::vector<std::string> & args, BraceExpander & exp) = 0;
};

class EnvEvaler : public Evaler
{
public:
  virtual EvalResult
  eval(Field * n, const std::vector<std::string> & args, BraceExpander & exp) override;
};

class RawEvaler : public Evaler
{
public:
  virtual EvalResult
  eval(Field * n, const std::vector<std::string> & args, BraceExpander & exp) override;
};

class ReplaceEvaler : public Evaler
{
public:
  virtual EvalResult
  eval(Field * n, const std::vector<std::string> & args, BraceExpander & exp) override;
};

class BraceExpander : public Walker
{
public:
  BraceExpander() {}
  void registerEvaler(const std::string & name, Evaler & ev);
  virtual void walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, Node * n);
  std::string expand(Field * n, const std::string & input);
  Field *
  findReferencedField(Field * n, const std::string & path, std::string * used_path = nullptr);
  const EvalResult & resolve(Field * n);
  ErrorMessage currentExpressionErrorMessage(Field * n, const std::string & message) const;
  Error currentExpressionError(Field * n, const std::string & message) const;

  std::vector<std::string> used;
  std::vector<ErrorMessage> errors;

private:
  enum class ResolveState
  {
    Unvisited,
    Resolving,
    Resolved
  };

  struct ResolveEntry
  {
    ResolveState state = ResolveState::Unvisited;
    EvalResult result;
  };

  EvalResult evaluate(Field * n, const BraceTemplate & expr);
  EvalResult evaluate(Field * n, const BraceExpr & expr);
  Error syntaxError(Field * n, const BraceParseError & error) const;
  Error expressionError(Field * n, const BraceExpr & expr, const std::string & message) const;
  ErrorMessage
  expressionErrorMessage(Field * n, const BraceExpr & expr, const std::string & message) const;
  std::map<std::string, Evaler *> _evalers;
  std::map<Field *, ResolveEntry> _resolved;
  std::vector<Field *> _resolving_stack;
  const BraceExpr * _active_expr = nullptr;
  ReplaceEvaler _replace;
};

} // namespace hit
