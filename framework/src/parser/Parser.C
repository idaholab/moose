//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseUtils.h"
#include "MooseInit.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "MooseTypes.h"
#include "CommandLine.h"
#include "SystemInfo.h"
#include "Parser.h"
#include "Units.h"

#include "libmesh/parallel.h"
#include "libmesh/fparser.hh"

// Regular expression includes
#include "pcrecpp.h"

// C++ includes
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>

std::string
FuncParseEvaler::eval(hit::Field * n, const std::list<std::string> & args, hit::BraceExpander & exp)
{
  std::string func_text;
  for (auto & s : args)
    func_text += s;
  auto n_errs = exp.errors.size();

  FunctionParser fp;
  fp.AddConstant("pi", libMesh::pi);
  fp.AddConstant("e", std::exp(Real(1)));
  std::vector<std::string> var_names;
  auto ret = fp.ParseAndDeduceVariables(func_text, var_names);
  if (ret != -1)
  {
    exp.errors.push_back(hit::errormsg(n, "fparse error: ", fp.ErrorMsg()));
    return n->val();
  }

  std::string errors;
  std::vector<double> var_vals;
  for (auto & var : var_names)
  {
    // recursively check all parent scopes for the needed variables
    hit::Node * curr = n;
    while ((curr = curr->parent()))
    {
      auto src = curr->find(var);
      if (src && src != n && src->type() == hit::NodeType::Field)
      {
        exp.used.push_back(hit::pathJoin({curr->fullpath(), var}));
        var_vals.push_back(curr->param<double>(var));
        break;
      }
    }

    if (curr == nullptr)
      exp.errors.push_back(hit::errormsg(
          n, "\n    no variable '", var, "' found for use in function parser expression"));
  }

  if (exp.errors.size() != n_errs)
    return n->val();

  std::stringstream ss;
  ss << std::setprecision(17) << fp.Eval(var_vals.data());

  // change kind only (not val)
  n->setVal(n->val(), hit::Field::Kind::Float);
  return ss.str();
}

std::string
UnitsConversionEvaler::eval(hit::Field * n,
                            const std::list<std::string> & args,
                            hit::BraceExpander & exp)
{
  std::vector<std::string> argv;
  argv.insert(argv.begin(), args.begin(), args.end());

  // no conversion, the expression currently only documents the units and passes through the value
  if (argv.size() == 2)
  {
    n->setVal(n->val(), hit::Field::Kind::Float);
    return argv[0];
  }

  // conversion
  if (argv.size() != 4 || (argv.size() >= 3 && argv[2] != "->"))
  {
    exp.errors.push_back(
        hit::errormsg(n,
                      "units error: Expected 4 arguments ${units number from_unit -> to_unit} or "
                      "2 arguments  ${units number unit}"));
    return n->val();
  }

  // get and check units
  auto from_unit = MooseUnits(argv[1]);
  auto to_unit = MooseUnits(argv[3]);
  if (!from_unit.conformsTo(to_unit))
  {
    exp.errors.push_back(hit::errormsg(n,
                                       "units error: ",
                                       argv[1],
                                       " (",
                                       from_unit,
                                       ") does not convert to ",
                                       argv[3],
                                       " (",
                                       to_unit,
                                       ")"));
    return n->val();
  }

  // parse number
  Real num = MooseUtils::convert<Real>(argv[0]);

  // convert units
  std::stringstream ss;
  ss << std::setprecision(17) << to_unit.convert(num, from_unit);

#ifndef NDEBUG
  mooseInfoRepeated(n->filename() + ':' + Moose::stringify(n->line()) + ':' +
                        Moose::stringify(n->column()) + ": Unit conversion ",
                    num,
                    ' ',
                    argv[1],
                    " -> ",
                    ss.str(),
                    ' ',
                    argv[3]);
#endif

  // change kind only (not val)
  n->setVal(n->val(), hit::Field::Kind::Float);
  return ss.str();
}

Parser::Parser() : _sections_read(false) {}

Parser::~Parser() {}

void
DupParamWalker ::walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n)
{
  std::string prefix = n->type() == hit::NodeType::Field ? "parameter" : "section";

  if (_have.count(fullpath) > 0)
  {
    auto existing = _have[fullpath];
    if (_duplicates.count(fullpath) == 0)
    {
      errors.push_back(
          hit::errormsg(existing, prefix, " '", fullpath, "' supplied multiple times"));
      _duplicates.insert(fullpath);
    }
    errors.push_back(hit::errormsg(n, prefix, " '", fullpath, "' supplied multiple times"));
  }
  _have[fullpath] = n;
}

void
CompileParamWalker::walk(const std::string & fullpath,
                         const std::string & /*nodepath*/,
                         hit::Node * n)
{
  if (n->type() == hit::NodeType::Field)
    _map[fullpath] = n;
}

void
OverrideParamWalker::walk(const std::string & fullpath,
                          const std::string & /*nodepath*/,
                          hit::Node * n)
{
  const auto it = _map.find(fullpath);
  if (it != _map.end())
    warnings.push_back(hit::errormsg(n,
                                     " Parameter '",
                                     fullpath,
                                     "' overrides the same parameter in ",
                                     it->second->filename(),
                                     ":",
                                     it->second->line()));
}

void
BadActiveWalker ::walk(const std::string & /*fullpath*/,
                       const std::string & /*nodepath*/,
                       hit::Node * section)
{
  auto actives = section->find("active");
  auto inactives = section->find("inactive");

  if (actives && inactives && actives->type() == hit::NodeType::Field &&
      inactives->type() == hit::NodeType::Field && actives->parent() == inactives->parent())
  {
    errors.push_back(
        hit::errormsg(section, "'active' and 'inactive' parameters both provided in section"));
    return;
  }

  // ensures we don't recheck deeper nesting levels
  if (actives && actives->type() == hit::NodeType::Field && actives->parent() == section)
  {
    auto vars = section->param<std::vector<std::string>>("active");
    std::string msg = "";
    for (auto & var : vars)
    {
      if (!section->find(var))
        msg += var + ", ";
    }
    if (msg.size() > 0)
    {
      msg = msg.substr(0, msg.size() - 2);
      errors.push_back(hit::errormsg(section,
                                     "variables listed as active (",
                                     msg,
                                     ") in section '",
                                     section->fullpath(),
                                     "' not found in input"));
    }
  }
  // ensures we don't recheck deeper nesting levels
  if (inactives && inactives->type() == hit::NodeType::Field && inactives->parent() == section)
  {
    auto vars = section->param<std::vector<std::string>>("inactive");
    std::string msg = "";
    for (auto & var : vars)
    {
      if (!section->find(var))
        msg += var + ", ";
    }
    if (msg.size() > 0)
    {
      msg = msg.substr(0, msg.size() - 2);
      errors.push_back(hit::errormsg(section,
                                     "variables listed as inactive (",
                                     msg,
                                     ") in section '",
                                     section->fullpath(),
                                     "' not found in input"));
    }
  }
}

hit::Node *
Parser::root()
{
  // normal input file is provided, front parser is set to parse input file
  if (_root)
    return _root.get();
  // no valid input file is provided, front parser is skipped
  else
    return nullptr;
}

void
Parser::parse(const std::vector<std::string> & input_filenames,
              const std::optional<std::string> & input_text)
{
  // Check that if the input_text string is provided, then there is only one filename to match
  if (input_text.has_value() && input_filenames.size() != 1)
    mooseError("If 'input_text' is provided, then 'input_filenames' must hold only one filename");

  // Save the filename
  _input_filenames = input_filenames;

  if (_input_filenames.size() > 1)
    mooseInfo("Merging inputs ", Moose::stringify(_input_filenames));

  // We don't (currently) want this option to propagate to other objects that need
  // the input paths. At least, we didn't in the past. So we're going to retain that
  // behavior for now by only changing it from the read here. In the future, we'll
  // probably want to make this more consistent.
  auto filenames = _input_filenames;
  std::string use_rel_paths_str =
      std::getenv("MOOSE_RELATIVE_FILEPATHS") ? std::getenv("MOOSE_RELATIVE_FILEPATHS") : "false";
  if (use_rel_paths_str == "0" || use_rel_paths_str == "false")
    for (auto & input_filename : filenames)
      input_filename = MooseUtils::realpath(input_filename);

  CompileParamWalker::ParamMap override_map;
  CompileParamWalker cpw(override_map);
  OverrideParamWalker opw(override_map);

  for (auto & input_filename : filenames)
  {
    // Parse the input text string if provided, otherwise read file from disk
    std::string input;
    std::string errmsg;
    if (input_text.has_value())
      input = input_text.value();
    else
    {
      MooseUtils::checkFileReadable(input_filename, true);
      std::ifstream f(input_filename);
      input = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    }

    try
    {
      std::unique_ptr<hit::Node> root(hit::parse(input_filename, input));
      hit::explode(root.get());
      DupParamWalker dw;
      root->walk(&dw, hit::NodeType::Field);
      if (!_root)
        _root = std::move(root);
      else
      {
        root->walk(&opw, hit::NodeType::Field);
        hit::merge(root.get(), _root.get());
      }

      for (auto & msg : dw.errors)
        errmsg += msg + "\n";

      _dw_errmsg.push_back(errmsg);
      _root->walk(&cpw, hit::NodeType::Field);
    }
    catch (hit::ParseError & err)
    {
      mooseError(err.what());
    }
  }

  // warn about overridden parameters in multiple inputs
  if (!opw.warnings.empty())
    mooseInfo(Moose::stringify(opw.warnings), "\n");

  // do as much error checking as early as possible so that errors are more useful instead
  // of surprising and disconnected from what caused them.
  BadActiveWalker bw;
  _root->walk(&bw, hit::NodeType::Section);

  for (auto & msg : bw.errors)
    _errmsg += msg + "\n";

  // Print parse errors related to bad active early
  if (_errmsg.size() > 0)
    mooseError(_errmsg);

  for (auto & msg : _dw_errmsg)
    if (msg.size() > 0)
      mooseError(msg);
}
