#ifndef HIT_BRACEEXPR
#define HIT_BRACEEXPR

#include <string>
#include <vector>
#include <list>
#include <map>
#include <cstdlib>

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
  virtual std::string eval(std::list<std::string> & args) = 0;
};

class EnvEvaler : public Evaler
{
public:
  virtual std::string eval(std::list<std::string> & args) override;
};

class RawEvaler : public Evaler
{
public:
  virtual std::string eval(std::list<std::string> & args) override;
};

class BraceExpander
{
public:
  void registerEvaler(const std::string & name, Evaler & ev);
  std::string expand(const std::string & input);

private:
  std::string expand(BraceNode & expr);

  std::map<std::string, Evaler *> _evalers;
};

#endif
