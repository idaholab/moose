
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <set>

#include "parse.h"

class Flags;
std::vector<std::string> parseOpts(int argc, char ** argv, Flags & flags);

int renameParam(int argc, char ** argv);
int findParam(int argc, char ** argv);
int validate(int argc, char ** argv);
int format(int argc, char ** argv);

int
main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cerr << "must specify a subcommand\n";
    return 1;
  }

  std::string subcmd(argv[1]);

  if (subcmd == "rename")
    return renameParam(argc - 2, argv + 2);
  else if (subcmd == "find")
    return findParam(argc - 2, argv + 2);
  else if (subcmd == "validate")
    return validate(argc - 2, argv + 2);
  else if (subcmd == "format")
    return format(argc - 2, argv + 2);

  std::cerr << "unrecognized subcommand '" << subcmd << "'\n";
  return 1;
}

struct Flag
{
  bool arg;
  bool have;
  std::string val;
  std::string help;
};

class Flags
{
public:
  void add(std::string name, std::string help, std::string def = "__NONE__")
  {
    if (def == "__NONE__")
      flags[name] = {false, false, "", help};
    else
      flags[name] = {true, false, def, help};
  }

  bool have(std::string flag) { return flags.count(flag) > 0 && flags[flag].have; }
  std::string val(std::string flag) { return flags[flag].val; }

  std::map<std::string, Flag> flags;
};

std::vector<std::string>
parseOpts(int argc, char ** argv, Flags & flags)
{
  int i = 0;
  for (; i < argc; i++)
  {
    std::string arg = argv[i];
    if (arg[0] != '-')
      break;

    std::string flagname = arg.substr(1);
    if (flagname[0] == '-')
      flagname = flagname.substr(1);

    if (flags.flags.count(flagname) == 0)
      throw std::runtime_error("unknown flag '" + arg);

    auto & flag = flags.flags[flagname];
    flag.have = true;
    if (flag.arg)
    {
      i++;
      flag.val = argv[i];
    }
  }

  std::vector<std::string> positional;
  for (; i < argc; i++)
    positional.push_back(argv[i]);
  return positional;
}

inline std::string
errormsg(std::string /*fname*/, hit::Node * /*n*/)
{
  return "";
}

template <typename T, typename... Args>
std::string
errormsg(std::string fname, hit::Node * n, T arg, Args... args)
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

class DupParamWalker : public hit::Walker
{
public:
  DupParamWalker(std::string fname) : _fname(fname) {}
  void walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override
  {
    std::string prefix = n->type() == hit::NodeType::Field ? "parameter" : "section";

    if (_have.count(fullpath) > 0)
    {
      auto existing = _have[fullpath];
      if (_duplicates.count(fullpath) == 0)
      {
        errors.push_back(
            errormsg(_fname, existing, prefix, " '", fullpath, "' supplied multiple times"));
        _duplicates.insert(fullpath);
      }
      errors.push_back(errormsg(_fname, n, prefix, " '", fullpath, "' supplied multiple times"));
    }
    _have[n->fullpath()] = n;
  }

  std::vector<std::string> errors;

private:
  std::string _fname;
  std::set<std::string> _duplicates;
  std::map<std::string, hit::Node *> _have;
};

int
renameParam(int argc, char ** argv)
{
  Flags flags;
  flags.add("i", "modify file(s) inplace");
  auto positional = parseOpts(argc, argv, flags);

  if (positional.size() < 3)
  {
    std::cerr << "rename expects 2 arguments: [src-path] [dst-path] [file]...";
    return 1;
  }

  std::string src(positional[0]);
  std::string dst(positional[1]);

  int ret = 0;
  for (int i = 2; i < positional.size(); i++)
  {
    std::string fname(positional[i]);
    std::ifstream f(fname);
    std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    hit::Node * root = nullptr;
    try
    {
      root = hit::parse(fname, input);
    }
    catch (std::exception & err)
    {
      std::cerr << err.what() << "\n";
      return 1;
    }

    auto n = root->find(src);
    if (n)
    {
      auto f = dynamic_cast<hit::Field *>(n);
      delete n;

      auto dstnode = new hit::Field(dst, f->kind(), f->val());
      auto dstnodetree = new hit::Section("");
      dstnodetree->addChild(dstnode);
      hit::explode(dstnodetree);
      hit::merge(dstnodetree, root);

      std::string updated = root->render();
      if (!flags.have("i"))
        std::cout << updated << "\n";
      else
      {
        std::ofstream f(fname);
        f << updated;
      }
    }
  }

  return ret;
}

int
findParam(int argc, char ** argv)
{
  Flags flags;
  flags.add("f", "only show file name");
  auto positional = parseOpts(argc, argv, flags);

  if (positional.size() < 2)
  {
    std::cerr << "find expects 2 or more arguments: [parameter-path] [file]...";
    return 1;
  }

  std::string srcpath(positional[0]);

  int ret = 0;
  for (int i = 1; i < positional.size(); i++)
  {
    std::string fname(positional[i]);
    std::ifstream f(fname);
    std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    hit::Node * root = nullptr;
    try
    {
      root = hit::parse(fname, input);
    }
    catch (std::exception & err)
    {
      std::cerr << err.what() << "\n";
      ret = 1;
      continue;
    }

    auto n = root->find(srcpath);
    if (n)
    {
      if (flags.have("f"))
        std::cout << fname << "\n";
      else
        std::cout << fname << ":" << n->line() << "\n";
    }
  }

  return ret;
}

int
format(int argc, char ** argv)
{
  Flags flags;
  flags.add("i", "modify file(s) inplace");
  auto positional = parseOpts(argc, argv, flags);

  if (positional.size() < 1)
  {
    std::cerr << "please pass in an input file argument\n";
    return 1;
  }

  int ret = 0;
  for (int i = 0; i < positional.size(); i++)
  {
    std::string fname(positional[i]);
    std::ifstream f(fname);
    std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    hit::Node * root = nullptr;
    try
    {
      root = hit::parse(fname, input);
    }
    catch (std::exception & err)
    {
      std::cerr << err.what() << "\n";
      ret = 1;
      continue;
    }

    if (!flags.have("i"))
      std::cout << root->render() << "\n";
    else
    {
      std::ofstream f(fname);
      f << root->render();
    }
  }

  return ret;
}

int
validate(int argc, char ** argv)
{
  if (argc < 1)
  {
    std::cerr << "please pass in an input file argument\n";
    return 1;
  }

  int ret = 0;
  for (int i = 0; i < argc; i++)
  {
    std::string fname(argv[i]);
    std::ifstream f(fname);
    std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    hit::Node * root = nullptr;
    try
    {
      root = hit::parse(fname, input);
    }
    catch (std::exception & err)
    {
      std::cout << err.what() << "\n";
      ret = 1;
      continue;
    }

    DupParamWalker w(fname);
    root->walk(&w, hit::NodeType::Field);
    for (auto & msg : w.errors)
      std::cout << msg << "\n";
  }
  return ret;
}
