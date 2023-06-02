#pragma once

#include "parse.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <sstream>

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
class BraceExpander;

class Evaler
{
public:
  virtual ~Evaler() {}
  virtual std::string eval(Field * n, const std::list<std::string> & args, BraceExpander & exp) = 0;
};

class EnvEvaler : public Evaler
{
public:
  virtual std::string
  eval(Field * n, const std::list<std::string> & args, BraceExpander & exp) override;
};

class RawEvaler : public Evaler
{
public:
  virtual std::string
  eval(Field * n, const std::list<std::string> & args, BraceExpander & exp) override;
};

class ReplaceEvaler : public Evaler
{
public:
  virtual std::string
  eval(Field * n, const std::list<std::string> & args, BraceExpander & exp) override;
};

class BraceExpander : public Walker
{
public:
  BraceExpander() {}
  void registerEvaler(const std::string & name, Evaler & ev);
  virtual void walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, Node * n);
  std::string expand(Field * n, const std::string & input);

  std::vector<std::string> used;
  std::vector<std::string> errors;

private:
  std::string expand(Field * n, BraceNode & expr);
  std::map<std::string, Evaler *> _evalers;
  ReplaceEvaler _replace;
};

} // namespace hit
