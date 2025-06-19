//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseUtils.h"
#include "MooseInit.h"
#include "MooseTypes.h"
#include "CommandLine.h"
#include "SystemInfo.h"
#include "Parser.h"
#include "Units.h"

#include "libmesh/parallel.h"
#include "libmesh/fparser.hh"

// C++ includes
#include <map>
#include <fstream>
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
    exp.errors.emplace_back(
        "fparse error: " + std::string(fp.ErrorMsg()) + " in '" + n->fullpath() + "'", n);
    return n->val();
  }

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
      exp.errors.emplace_back("no variable '" + var +
                                  "' found for use in function parser expression in '" +
                                  n->fullpath() + "'",
                              n);
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
    exp.errors.emplace_back("units error: Expected 4 arguments ${units number from_unit -> "
                            "to_unit} or 2 arguments ${units number unit} in '" +
                                n->fullpath() + "'",
                            n);
    return n->val();
  }

  // get and check units
  auto from_unit = MooseUnits(argv[1]);
  auto to_unit = MooseUnits(argv[3]);
  if (!from_unit.conformsTo(to_unit))
  {
    std::ostringstream err;
    err << "units error: " << argv[1] << " (" << from_unit << ") does not convert to " << argv[3]
        << " (" << to_unit << ") in '" << n->fullpath() << "'";
    exp.errors.emplace_back(err.str(), n);
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

Parser::Parser(const std::vector<std::string> & input_filenames,
               const std::optional<std::vector<std::string>> & input_text /* = {} */)
  : _root(nullptr),
    _input_filenames(input_filenames),
    _input_text(input_text ? *input_text : std::vector<std::string>()),
    _cli_root(nullptr),
    _throw_on_error(false)
{
  if (input_text && _input_filenames.size() != input_text->size())
    mooseError("Parser: Input text not the same length as input filenames");
}

Parser::Parser(const std::string & input_filename,
               const std::optional<std::string> & input_text /* = {} */)
  : Parser(std::vector<std::string>{input_filename},
           input_text ? std::optional<std::vector<std::string>>({*input_text})
                      : std::optional<std::vector<std::string>>())
{
}

void
DupParamWalker::walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n)
{
  const auto it = _have.try_emplace(fullpath, n);
  if (!it.second)
  {
    const std::string type = n->type() == hit::NodeType::Field ? "parameter" : "section";
    const std::string error = type + " '" + fullpath + "' supplied multiple times";

    // Don't warn multiple times (will happen if we find it three+ times)
    const auto existing = it.first->second;
    if (std::find_if(errors.begin(),
                     errors.end(),
                     [&existing](const auto & err)
                     { return err.node == existing; }) == errors.end())
      errors.emplace_back(error, existing);

    errors.emplace_back(error, n);
  }
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
BadActiveWalker ::walk(const std::string & fullpath,
                       const std::string & /*nodepath*/,
                       hit::Node * section)
{
  auto actives = section->find("active");
  auto inactives = section->find("inactive");

  if (actives && inactives && actives->type() == hit::NodeType::Field &&
      inactives->type() == hit::NodeType::Field && actives->parent() == inactives->parent())
  {
    errors.emplace_back(
        "'active' and 'inactive' parameters both provided in section '" + fullpath + "'", section);
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
      errors.emplace_back("variables listed as active (" + msg + ") in section '" +
                              section->fullpath() + "' not found in input",
                          section);
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
      errors.emplace_back("variables listed as inactive (" + msg + ") in section '" +
                              section->fullpath() + "' not found in input",
                          section);
    }
  }
}

class FindAppWalker : public hit::Walker
{
public:
  void
  walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, hit::Node * n) override
  {
    if (n && n->type() == hit::NodeType::Field && n->fullpath() == "Application/type")
      _app_type = n->param<std::string>();
  }
  const std::optional<std::string> & getApp() { return _app_type; };

private:
  std::optional<std::string> _app_type;
};

void
Parser::setCommandLineParams(const std::vector<std::string> & params)
{
  mooseAssert(!_command_line_params, "Already set");
  _command_line_params = params;
}

const std::string &
Parser::getLastInputFileName() const
{
  if (_input_filenames.empty())
    mooseError("Parser::getLastInputFileName(): No inputs are set");
  return _input_filenames.back();
}

Parser::Error::Error(const std::vector<hit::ErrorMessage> & error_messages)
  : hit::Error(error_messages)
{
}

void
Parser::parse()
{
  mooseAssert(!_root && !_cli_root, "Has already parsed");

  if (getInputFileNames().size() > 1)
    mooseInfo("Merging inputs ", Moose::stringify(getInputFileNames()));

  // Correct filenames (default is to use real path)
  const std::string use_rel_paths_str =
      std::getenv("MOOSE_RELATIVE_FILEPATHS") ? std::getenv("MOOSE_RELATIVE_FILEPATHS") : "false";
  const auto use_real_paths = use_rel_paths_str == "0" || use_rel_paths_str == "false";
  std::vector<std::string> filenames;
  for (const auto & filename : getInputFileNames())
    filenames.push_back(use_real_paths ? MooseUtils::realpath(filename) : filename);

  // Load each input file if text was not provided
  if (_input_text.empty())
    for (const auto & filename : filenames)
    {
      MooseUtils::checkFileReadable(filename, true);
      std::ifstream f(filename);
      _input_text.push_back(
          std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()));
    }

  CompileParamWalker::ParamMap override_map;
  CompileParamWalker cpw(override_map);
  OverrideParamWalker opw(override_map);

  // Errors from the duplicate param walker, ran within each input
  // independently first
  std::vector<hit::ErrorMessage> dw_errors;

  for (const auto i : index_range(getInputFileNames()))
  {
    const auto & filename = filenames[i];
    const auto & input = getInputText()[i];

    try
    {
      // provide stream to hit parse function to capture any syntax errors,
      // set parser root node, then throw those errors if any were captured
      std::vector<hit::ErrorMessage> syntax_errors;
      std::unique_ptr<hit::Node> root(hit::parse(filename, input, &syntax_errors));

      DupParamWalker dw;
      root->walk(&dw, hit::NodeType::Field);
      appendErrorMessages(dw_errors, dw.errors);

      if (!queryRoot())
        _root = std::move(root);
      else
      {
        root->walk(&opw, hit::NodeType::Field);
        hit::merge(root.get(), &getRoot());
      }

      if (!syntax_errors.empty())
        throw Parser::Error(syntax_errors);

      getRoot().walk(&cpw, hit::NodeType::Field);
    }
    catch (hit::Error & err)
    {
      parseError(err.error_messages);
    }
  }

  // warn about overridden parameters in multiple inputs
  if (!opw.warnings.empty())
    mooseInfo(Moose::stringify(opw.warnings), "\n");

  // If we don't have a root (allow no input files),
  // create an empty one
  if (!queryRoot())
    _root.reset(hit::parse("EMPTY", ""));

  {
    BadActiveWalker bw;
    getRoot().walk(&bw, hit::NodeType::Section);
    if (bw.errors.size())
      parseError(bw.errors);
  }

  {
    FindAppWalker fw;
    getRoot().walk(&fw, hit::NodeType::Field);
    if (fw.getApp())
      setAppType(*fw.getApp());
  }

  // Duplicate parameter errors (within each input file)
  if (dw_errors.size())
    parseError(dw_errors);

  // Merge in command line HIT arguments
  const auto joined_params =
      _command_line_params ? MooseUtils::stringJoin(*_command_line_params) : "";
  try
  {
    _cli_root.reset(hit::parse("CLI_ARGS", joined_params));
    hit::merge(&getCommandLineRoot(), &getRoot());
  }
  catch (hit::Error & err)
  {
    parseError(err.error_messages);
  }

  std::vector<hit::ErrorMessage> errors;

  // expand ${bla} parameter values and mark/include variables
  // used in expansion as "used" (obtained later by the Builder
  // with getExtractedVars())
  {
    hit::RawEvaler raw;
    hit::EnvEvaler env;
    hit::ReplaceEvaler repl;
    FuncParseEvaler fparse_ev;
    UnitsConversionEvaler units_ev;
    hit::BraceExpander exw;
    exw.registerEvaler("raw", raw);
    exw.registerEvaler("env", env);
    exw.registerEvaler("fparse", fparse_ev);
    exw.registerEvaler("replace", repl);
    exw.registerEvaler("units", units_ev);
    getRoot().walk(&exw);
    for (auto & var : exw.used)
      _extracted_vars.insert(var);
    Parser::appendErrorMessages(errors, exw.errors);
  }

  // Collect duplicate parameters now that we've merged inputs
  {
    DupParamWalker dw;
    getRoot().walk(&dw, hit::NodeType::Field);
    Parser::appendErrorMessages(errors, dw.errors);
  }

  // Check bad active now that we've merged inputs
  {
    BadActiveWalker bw;
    getRoot().walk(&bw, hit::NodeType::Section);
    Parser::appendErrorMessages(errors, bw.errors);
  }

  if (errors.size())
    parseError(errors);
}

hit::Node &
Parser::getRoot()
{
  if (!queryRoot())
    mooseError("Parser::getRoot(): root is not set");
  return *queryRoot();
}

const hit::Node &
Parser::getCommandLineRoot() const
{
  if (!queryCommandLineRoot())
    mooseError("Parser::getCommandLineRoot(): command line root is not set");
  return *queryCommandLineRoot();
}

hit::Node &
Parser::getCommandLineRoot()
{
  return const_cast<hit::Node &>(std::as_const(*this).getCommandLineRoot());
}

void
Parser::appendErrorMessages(std::vector<hit::ErrorMessage> & to,
                            const std::vector<hit::ErrorMessage> & from)
{
  to.insert(to.end(), from.begin(), from.end());
}

void
Parser::appendErrorMessages(std::vector<hit::ErrorMessage> & to, const hit::Error & error)
{
  appendErrorMessages(to, error.error_messages);
}

std::string
Parser::joinErrorMessages(const std::vector<hit::ErrorMessage> & error_messages)
{
  std::vector<std::string> values;
  for (const auto & em : error_messages)
    values.push_back(em.prefixed_message);
  return MooseUtils::stringJoin(values, "\n");
}

void
Parser::parseError(std::vector<hit::ErrorMessage> messages) const
{
  // Few things about command line arguments...
  // 1. We don't care to add line and column context for CLI args, because
  //    it doesn't make sense. We go from the full CLI args and pull out
  //    the HIT parameters so "line" 1 might not even be command line
  //    argument 1. So, remove line/column context from all CLI args.
  // 2. Whenever we have a parameter in input that then gets overridden
  //    by a command line argument, under the hood we're merging two
  //    different HIT trees. However, WASP doesn't currently update the
  //    "filename" context for the updated parameter. Which means that
  //    a param that is in input and then overridden by CLI will have
  //    its location as in input. Which isn't true. So we get around this
  //    by searching the independent CLI args tree for params that we have
  //    errors for. If the associated path is also in CLI args, we manually
  //    set its error to come from CLI args. This should be fixed in
  //    the future with a WASP update.
  for (auto & em : messages)
    if (em.node && queryCommandLineRoot())
      if (const auto cli_node = getCommandLineRoot().find(em.node->fullpath()))
        em = hit::ErrorMessage(em.message, "CLI_ARGS");

  if (_throw_on_error)
    throw Parser::Error(messages);
  else
    mooseError(joinErrorMessages(messages));
}
