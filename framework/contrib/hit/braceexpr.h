#ifndef HIT_BRACEEXPR
#define HIT_BRACEEXPR

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
    ss << fname << ":" << n->line() << ": ";
  else if (fname.size() > 0)
    ss << fname << ":0: ";
  ss << arg;
  ss << errormsg("", nullptr, args...);
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

class Evaler
{
public:
  virtual std::string eval(std::list<std::string> & args) const = 0;
};

class EnvEvaler : public Evaler
{
public:
  virtual std::string eval(std::list<std::string> & args) const override;
};

class RawEvaler : public Evaler
{
public:
  virtual std::string eval(std::list<std::string> & args) const override;
};

class BraceExpander
{
public:
  void registerEvaler(const std::string & name, const Evaler & ev);
  std::string expand(const std::string & input);

private:
  std::string expand(BraceNode & expr);

  std::map<std::string, const Evaler *> _evalers;
};

// Expands ${...} substitution expressions with variable values from the tree.
class ExpandWalker : public Walker
{
public:
  ExpandWalker(std::string fname, BraceExpander& expander);
  virtual void walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, hit::Node * n);

  std::vector<std::string> used;
  std::vector<std::string> errors;

private:
  std::string _fname;
  BraceExpander& _expander;
};

} // namespace hit

#endif
