
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

#include "parse.h"

class MyWalker : public hit::Walker
{
public:
  void walk(const std::string & fullpath, const std::string & nodepath, hit::Node * f)
  {
    std::cout << "    path=" << fullpath << "/" << nodepath << "\n";
  }
};

class DupSectionWalker : public hit::Walker
{
public:
  void walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, hit::Node * n)
  {
    if (have.count(n->fullpath()) > 0)
    {
      if (duplicates.count(n->fullpath()) == 0)
        duplicates[n->fullpath()].push_back(have[n->fullpath()]);
      duplicates[n->fullpath()].push_back(n);
    }
    have[n->fullpath()] = n;
  }
  std::map<std::string, std::vector<hit::Node *>> duplicates;

private:
  std::map<std::string, hit::Node *> have;
};

int
checkfiles(int argc, char ** argv)
{
  int ret = 0;
  if (argc > 1)
  {
    for (int i = 1; i < argc; i++)
    {
      std::string fname(argv[i]);

      std::ifstream f(fname);
      std::stringstream ss;
      ss << f.rdbuf();
      std::string input = ss.str();

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

      DupSectionWalker w;
      root->walk(&w, hit::NodeType::Section);

      for (auto & it : w.duplicates)
      {
        for (auto sec : it.second)
        {
          ret = 1;
          std::cout << fname << ":" << sec->line() << ": duplicate section name '" << it.first
                    << "'\n";
        }
      }
    }
    return ret;
  }
  else
  {
    std::cout << "please pass in an input file argument\n";
    return 1;
  }
}

int
main(int argc, char ** argv)
{
  return checkfiles(argc, argv);

  if (argc > 1)
  {
    std::string fname(argv[1]);

    std::ifstream f(fname);
    std::stringstream ss;
    ss << f.rdbuf();
    std::string input = ss.str();

    hit::Node * root = hit::parse(fname, input);

    std::cout << "re-render file from AST:\n" << root->render();

    // manually hop around the AST by indexing nodes' children
    hit::Node * n = root->find("Kernels/diff");
    for (auto child : n->children())
      std::cout << "child=" << child->fullpath() << "\n";

    // search for param values in various ways
    std::cout << root->find("Kernels/diff/type")->param<std::string>() << "\n";
    std::cout << root->children()[2]->find("diff")->param<std::string>("type") << "\n";
    std::cout << root->param<std::string>("Kernels/diff/type") << "\n";

    // walk the entire tree with your custom code
    MyWalker w;
    std::cout << "beginning tree walk:\n";
    root->walk(&w);

    delete root;
  }

  auto input = "[foo] [bar] [] bar/boo=43 bam/boo=44 [] foo/bar/baz=42 vec='-42 43.2 0   44\n45\n  "
               "  \t  46'";
  auto root = hit::parse("testinput", input);
  std::cout << "re-render testinput from AST:\n" << root->render();
  hit::explode(root);
  std::cout << "re-render testinput from explodedAST:\n" << root->render();

  auto vec = root->param<std::vector<std::string>>("vec");
  for (int i = 0; i < vec.size(); i++)
    std::cout << "vec[" << i << "]=" << vec[i] << "\n";

  return 0;
}
