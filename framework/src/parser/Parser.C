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
#include "InputParameters.h"
#include "ActionFactory.h"
#include "Action.h"
#include "Factory.h"
#include "MooseObjectAction.h"
#include "ActionWarehouse.h"
#include "EmptyAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "Executioner.h"
#include "MooseApp.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "MultiApp.h"
#include "GlobalParamsAction.h"
#include "SyntaxTree.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "MooseTypes.h"
#include "CommandLine.h"
#include "JsonSyntaxTree.h"
#include "SystemInfo.h"
#include "MooseUtils.h"
#include "Conversion.h"
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

  // change kind only (not val)
  n->setVal(n->val(), hit::Field::Kind::Float);
  return ss.str();
}

Parser::Parser(MooseApp & app, ActionWarehouse & action_wh)
  : ConsoleStreamInterface(app),
    _app(app),
    _factory(app.getFactory()),
    _action_wh(action_wh),
    _action_factory(app.getActionFactory()),
    _syntax(_action_wh.syntax()),
    _syntax_formatter(nullptr),
    _sections_read(false),
    _current_params(nullptr),
    _current_error_stream(nullptr)
{
}

Parser::~Parser() {}

bool
isSectionActive(std::string path, hit::Node * root)
{
  hit::Node * n = root->find(path);
  while (n)
  {
    hit::Node * section = n->parent();
    if (section)
    {
      auto actives = section->find("active");
      auto inactives = section->find("inactive");

      // only check current level, not nested ones
      if (actives && actives->type() == hit::NodeType::Field && actives->parent() == section)
      {
        auto vars = section->param<std::vector<std::string>>("active");
        bool have_var = false;
        for (auto & var : vars)
          if (n->path() == hit::pathNorm(var))
            have_var = true;
        if (!have_var)
          return false;
      }
      // only check current level, not nested ones
      if (inactives && inactives->type() == hit::NodeType::Field && inactives->parent() == section)
      {
        auto vars = section->param<std::vector<std::string>>("inactive");
        for (auto & var : vars)
          if (n->path() == hit::pathNorm(var))
            return false;
      }
    }
    n = section;
  }
  return true;
}

class DupParamWalker : public hit::Walker
{
public:
  DupParamWalker() {}
  void walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override
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

  std::vector<std::string> errors;

private:
  std::set<std::string> _duplicates;
  std::map<std::string, hit::Node *> _have;
};

class CompileParamWalker : public hit::Walker
{
public:
  typedef std::map<std::string, hit::Node *> ParamMap;
  CompileParamWalker(ParamMap & map) : _map(map) {}
  void walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override
  {
    if (n->type() == hit::NodeType::Field)
      _map[fullpath] = n;
  }

private:
  ParamMap & _map;
};

class OverrideParamWalker : public hit::Walker
{
public:
  OverrideParamWalker(const CompileParamWalker::ParamMap & map) : _map(map) {}
  void walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override
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

  std::vector<std::string> warnings;

private:
  const CompileParamWalker::ParamMap & _map;
};

std::vector<std::string>
findSimilar(std::string param, std::vector<std::string> options)
{
  std::vector<std::string> candidates;
  if (options.size() == 0)
    return candidates;

  int mindist = MooseUtils::levenshteinDist(options[0], param);
  for (auto & opt : options)
  {
    int dist = MooseUtils::levenshteinDist(opt, param);
    // magic number heuristics to get similarity distance cutoff
    int dist_cutoff = 1 + param.size() / 5;
    if (dist > dist_cutoff || dist > mindist)
      continue;

    if (dist < mindist)
    {
      mindist = dist;
      candidates.clear();
    }
    candidates.push_back(opt);
  }
  return candidates;
}

std::vector<std::string>
Parser::listValidParams(std::string & section_name)
{
  bool dummy;
  std::string registered_identifier = _syntax.isAssociated(section_name, &dummy);
  auto iters = _syntax.getActions(registered_identifier);

  std::vector<std::string> paramlist;
  for (auto it = iters.first; it != iters.second; ++it)
  {
    auto params = _action_factory.getValidParams(it->second._action);
    for (const auto & it : params)
      paramlist.push_back(it.first);
  }
  return paramlist;
}

class UnusedWalker : public hit::Walker
{
public:
  UnusedWalker(std::set<std::string> used, Parser & p) : _used(used), _parser(p) {}

  void walk(const std::string & fullpath, const std::string & nodename, hit::Node * n) override
  {
    // the line() > 0 check allows us to skip nodes that were merged into this tree (i.e. CLI
    // args) because their unused params are checked+reported independently of the ones in the
    // main tree.
    if (!_used.count(fullpath) && nodename != "active" && nodename != "inactive" &&
        isSectionActive(fullpath, n->root()) && n->line() > 0)
    {
      auto section_name = fullpath.substr(0, fullpath.rfind("/"));
      auto paramlist = _parser.listValidParams(section_name);
      auto candidates = findSimilar(nodename, paramlist);
      if (candidates.size() > 0)
        errors.push_back(hit::errormsg(
            n, "unused parameter '", fullpath, "'\n", "      Did you mean '", candidates[0], "'?"));
      else
        errors.push_back(hit::errormsg(n, "unused parameter '", fullpath, "'"));
    }
  }

  std::vector<std::string> errors;

private:
  std::set<std::string> _used;
  Parser & _parser;
};

class BadActiveWalker : public hit::Walker
{
public:
  BadActiveWalker() {}
  void walk(const std::string & /*fullpath*/,
            const std::string & /*nodepath*/,
            hit::Node * section) override
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
  std::vector<std::string> errors;
};

std::string
Parser::getPrimaryFileName(bool stripLeadingPath) const
{
  if (!stripLeadingPath)
    return _input_filenames.back();

  std::string filename;
  size_t pos = _input_filenames.back().find_last_of('/');
  if (pos != std::string::npos)
    filename = _input_filenames.back().substr(pos + 1);
  else
    filename = _input_filenames.back();

  return filename;
}

void
Parser::walkRaw(std::string /*fullpath*/, std::string /*nodepath*/, hit::Node * n)
{
  InputParameters active_list_params = Action::validParams();
  InputParameters params = EmptyAction::validParams();

  std::string section_name = n->fullpath();
  std::string curr_identifier = n->fullpath();

  // Before we retrieve any actions or build any objects, make sure that the section they are in
  // is active
  if (!isSectionActive(curr_identifier, _root.get()))
    return;

  // Extract the block parameters before constructing the action
  // There may be more than one Action registered for a given section in which case we need to
  // build them all
  bool is_parent;
  std::string registered_identifier = _syntax.isAssociated(section_name, &is_parent);

  // We need to retrieve a list of Actions associated with the current identifier
  auto iters = _syntax.getActions(registered_identifier);
  if (iters.first == iters.second)
  {
    _errmsg += hit::errormsg(n,
                             "section '[",
                             curr_identifier,
                             "]' does not have an associated \"Action\".\n Common causes:\n"
                             "- you misspelled the Action/section name\n"
                             "- the app you are running does not support this Action/syntax") +
               "\n";
    return;
  }

  for (auto it = iters.first; it != iters.second; ++it)
  {
    if (is_parent)
      continue;
    if (_syntax.isDeprecatedSyntax(registered_identifier))
      mooseDeprecated(
          hit::errormsg(n, _syntax.deprecatedActionSyntaxMessage(registered_identifier)));

    params = _action_factory.getValidParams(it->second._action);

    params.set<ActionWarehouse *>("awh") = &_action_wh;

    extractParams(curr_identifier, params);

    // Add the parsed syntax to the parameters object for consumption by the Action
    params.set<std::string>("task") = it->second._task;
    params.set<std::string>("registered_identifier") = registered_identifier;
    params.blockLocation() = n->filename() + ":" + std::to_string(n->line());
    params.blockFullpath() = n->fullpath();

    if (!(params.have_parameter<bool>("isObjectAction") && params.get<bool>("isObjectAction")))
      params.set<std::vector<std::string>>("control_tags")
          .push_back(MooseUtils::baseName(curr_identifier));

    // Create the Action
    std::shared_ptr<Action> action_obj =
        _action_factory.create(it->second._action, curr_identifier, params);

    {
      // extract the MooseObject params if necessary
      std::shared_ptr<MooseObjectAction> object_action =
          std::dynamic_pointer_cast<MooseObjectAction>(action_obj);
      if (object_action)
      {
        object_action->getObjectParams().blockLocation() = params.blockLocation();
        object_action->getObjectParams().blockFullpath() = params.blockFullpath();
        extractParams(curr_identifier, object_action->getObjectParams());
        object_action->getObjectParams()
            .set<std::vector<std::string>>("control_tags")
            .push_back(MooseUtils::baseName(curr_identifier));
      }
    }

    // add it to the warehouse
    _action_wh.addActionBlock(action_obj);
  }
}

void
Parser::walk(const std::string & fullpath, const std::string & nodepath, hit::Node * n)
{
  // skip sections that were manually processed first.
  for (auto & sec : _secs_need_first)
    if (nodepath == sec)
      return;
  walkRaw(fullpath, nodepath, n);
}

std::string
Parser::hitCLIFilter(std::string appname, const std::vector<std::string> & argv)
{
  std::string hit_text;
  bool afterDoubleDash = false;
  for (std::size_t i = 1; i < argv.size(); i++)
  {
    std::string arg(argv[i]);

    // all args after a "--" are hit parameters
    if (arg == "--")
    {
      afterDoubleDash = true;
      continue;
    } // otherwise try to guess if a hit params have started by looking for "=" and "/"
    else if (arg.find("=", 0) != std::string::npos)
      afterDoubleDash = true;

    // skip over args that don't look like or are before hit parameters
    if (!afterDoubleDash)
      continue;
    // skip arguments with no equals sign
    if (arg.find("=", 0) == std::string::npos)
      continue;
    // skip cli flags (i.e. start with dash)
    if (arg.find("-", 0) == 0)
      continue;
    if (appname == "main")
    {
      auto pos = arg.find(":", 0);
      if (pos == 0) // trim leading colon
        arg = arg.substr(pos + 1, arg.size() - pos - 1);
      else if (pos != std::string::npos && pos < arg.find("=", 0)) // param is for non-main subapp
        continue;
    }
    else // app we are loading is a multiapp subapp
    {
      std::string name;
      std::string num;
      pcrecpp::RE("(.*?)"  // Match the multiapp name
                  "(\\d+)" // math the multiapp number
                  )
          .FullMatch(appname, &name, &num);
      auto pos = arg.find(":", 0);
      if (pos == 0)
        ; // cli param is ":" prefixed meaning global for all main+subapps
      else if (pos == std::string::npos) // param is for main app - skip
        continue;
      else if (arg.find(":", pos + 1) != std::string::npos) // param is for a nested multiapp - skip
        continue;
      else if (arg.substr(0, pos) != appname &&
               arg.substr(0, pos) != name) // param is for different multiapp - skip
      {
        _app.commandLine()->markHitParam(i);
        continue;
      }
      arg = arg.substr(pos + 1, arg.size() - pos - 1); // trim off subapp name prefix
    }

    try
    {
      hit::check("CLI_ARG", arg);
      hit_text += " " + arg;
      // handle case where bash ate quotes around an empty string after the "="
      if (arg.find("=", 0) == arg.size() - 1)
        hit_text += "''";
      _app.commandLine()->markHitParamUsed(i);
    }
    catch (hit::ParseError & err)
    {
      // bash might have eaten quotes around a hit string value or vector
      // so try quoting after the "=" and reparse
      auto quoted = arg;
      auto pos = quoted.find("=", 0);
      if (pos != std::string::npos)
        quoted = arg.substr(0, pos + 1) + "'" + arg.substr(pos + 1, quoted.size() - pos) + "'";
      try
      {
        hit::check("CLI_ARG", quoted);
        hit_text += " " + quoted;
        _app.commandLine()->markHitParamUsed(i);
      }
      catch (hit::ParseError & err)
      {
        mooseError("invalid hit in arg '", arg, "': ", err.what());
      }
    }
  }
  return hit_text;
}

void
Parser::parse(const std::vector<std::string> & input_filenames)
{
  // Save the filename
  _input_filenames = input_filenames;
  if (_input_filenames.empty())
    mooseError("No input files specified.");
  if (_input_filenames.size() > 1)
    mooseInfo("Merging inputs ", Moose::stringify(_input_filenames));

  std::string use_rel_paths_str =
      std::getenv("MOOSE_RELATIVE_FILEPATHS") ? std::getenv("MOOSE_RELATIVE_FILEPATHS") : "false";
  if (use_rel_paths_str == "0" || use_rel_paths_str == "false")
    for (auto & input_filename : _input_filenames)
      input_filename = MooseUtils::realpath(input_filename);

  CompileParamWalker::ParamMap override_map;
  CompileParamWalker cpw(override_map);
  OverrideParamWalker opw(override_map);

  for (auto & input_filename : _input_filenames)
  {
    MooseUtils::checkFileReadable(input_filename, true);

    std::ifstream f(input_filename);
    std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    try
    {
      std::unique_ptr<hit::Node> root(hit::parse(input_filename, input));
      hit::explode(root.get());

      if (!_root)
        _root = std::move(root);
      else
      {
        root->walk(&opw, hit::NodeType::Field);
        hit::merge(root.get(), _root.get());
      }

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

  // add in command line arguments
  try
  {
    auto cli_input = hitCLIFilter(_app.name(), _app.commandLine()->getArguments());
    _cli_root.reset(hit::parse("CLI_ARGS", cli_input));
    hit::explode(_cli_root.get());
    hit::merge(_cli_root.get(), _root.get());
  }
  catch (hit::ParseError & err)
  {
    mooseError(err.what());
  }

  // expand ${bla} parameter values and mark/include variables used in expansions as "used".  This
  // MUST occur before parameter extraction - otherwise parameters will get wrong values.
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
  _root->walk(&exw);
  for (auto & var : exw.used)
    _extracted_vars.insert(var);
  for (auto & msg : exw.errors)
    _errmsg += msg + "\n";

  // do as much error checking as early as possible so that errors are more useful instead
  // of surprising and disconnected from what caused them.
  DupParamWalker dw;
  BadActiveWalker bw;
  _root->walk(&dw, hit::NodeType::Field);
  _root->walk(&bw, hit::NodeType::Section);
  for (auto & msg : dw.errors)
    _errmsg += msg + "\n";
  for (auto & msg : bw.errors)
    _errmsg += msg + "\n";

  // Print parse errors related to brace expansion early
  if (_errmsg.size() > 0)
    mooseError(_errmsg);

  // There are a few order dependent actions that have to be built first in
  // order for the parser and application to function properly:
  //
  // SetupDebugAction: This action can contain an option for monitoring the parser progress. It must
  //                   be parsed first to capture all of the parsing output.
  //
  // GlobalParamsAction: This action is checked during the parameter extraction routines of all
  //                     subsequent blocks. It must be parsed early since it must exist during
  //                     subsequent parameter extraction.
  //
  // DynamicObjectRegistration: This action must be built before any MooseObjectActions are built.
  //                            This is because we retrieve valid parameters from the Factory
  //                            during parse time. Objects must be registered before
  //                            validParameters can be retrieved.
  auto syntax = _syntax.getSyntaxByAction("SetupDebugAction");
  std::copy(syntax.begin(), syntax.end(), std::back_inserter(_secs_need_first));

  syntax = _syntax.getSyntaxByAction("GlobalParamsAction");
  std::copy(syntax.begin(), syntax.end(), std::back_inserter(_secs_need_first));

  syntax = _syntax.getSyntaxByAction("DynamicObjectRegistrationAction");
  std::copy(syntax.begin(), syntax.end(), std::back_inserter(_secs_need_first));

  // walk all the sections extracting paramters from each into InputParameters objects
  for (auto & sec : _secs_need_first)
  {
    auto n = _root->find(sec);
    if (n)
      walkRaw(n->parent()->fullpath(), n->path(), n);
  }
  _root->walk(this, hit::NodeType::Section);

  if (_errmsg.size() > 0)
    mooseError(_errmsg);
}

// Checks the input and the way it has been used and emits any errors/warnings.
// This has to be a separate function because for we don't know if some parameters were unused
// until all the multiapps/subapps have been fully initialized - which isn't complete until
// *after* all the other member functions on Parser have been run.  So this is here to be
// externally called at the right time.
void
Parser::errorCheck(const Parallel::Communicator & comm, bool warn_unused, bool err_unused)
{
  // this if guard is important in case the simulation was not configured via parsed input text -
  // e.g.  configured programatically.
  if (!_root || !_cli_root)
    return;

  UnusedWalker uw(_extracted_vars, *this);
  UnusedWalker uwcli(_extracted_vars, *this);

  _root->walk(&uw);
  _cli_root->walk(&uwcli);

  auto cli = _app.commandLine();
  if (warn_unused)
  {
    for (auto arg : cli->unused(comm))
      _warnmsg += hit::errormsg("CLI_ARG",
                                nullptr,
                                "unused command line parameter '",
                                cli->getArguments()[arg],
                                "'") +
                  "\n";
    for (auto & msg : uwcli.errors)
      _warnmsg += msg + "\n";
    for (auto & msg : uw.errors)
      _warnmsg += msg + "\n";
  }
  else if (err_unused)
  {
    for (auto arg : cli->unused(comm))
      _errmsg += hit::errormsg("CLI_ARG",
                               nullptr,
                               "unused command line parameter '",
                               cli->getArguments()[arg],
                               "'") +
                 "\n";
    for (auto & msg : uwcli.errors)
      _errmsg += msg + "\n";
    for (auto & msg : uw.errors)
      _errmsg += msg + "\n";
  }

  if (_warnmsg.size() > 0)
    mooseUnused(_warnmsg);
  if (_errmsg.size() > 0)
    mooseError(_errmsg);
}

void
Parser::initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode)
{
  switch (type)
  {
    case INPUT_FILE:
      _syntax_formatter = std::make_unique<InputFileFormatter>(dump_mode);
      break;
    case YAML:
      _syntax_formatter = std::make_unique<YAMLFormatter>(dump_mode);
      break;
    default:
      mooseError("Unrecognized Syntax Formatter requested");
      break;
  }
}

void
Parser::buildJsonSyntaxTree(JsonSyntaxTree & root) const
{
  std::vector<std::pair<std::string, Syntax::ActionInfo>> all_names;

  for (const auto & iter : _syntax.getAssociatedTypes())
    root.addSyntaxType(iter.first, iter.second);

  for (const auto & iter : _syntax.getAssociatedActions())
  {
    Syntax::ActionInfo act_info = iter.second;
    /**
     * If the task is nullptr that means we need to figure out which task goes with this syntax for
     * the purpose of building the Moose Object part of the tree. We will figure this out by asking
     * the ActionFactory for the registration info.
     */
    if (act_info._task == "")
      act_info._task = _action_factory.getTaskName(act_info._action);

    all_names.push_back(std::make_pair(iter.first, act_info));
  }

  for (const auto & act_names : all_names)
  {
    const auto & act_info = act_names.second;
    const std::string & action = act_info._action;
    const std::string & task = act_info._task;
    const std::string act_name = act_names.first;
    InputParameters action_obj_params = _action_factory.getValidParams(action);
    bool params_added = root.addParameters("",
                                           act_name,
                                           false,
                                           action,
                                           true,
                                           &action_obj_params,
                                           _syntax.getLineInfo(act_name, action, ""),
                                           "");

    if (params_added)
    {
      auto tasks = _action_factory.getTasksByAction(action);
      for (auto & t : tasks)
      {
        auto info = _action_factory.getLineInfo(action, t);
        root.addActionTask(act_name, action, t, info);
      }
    }

    /**
     * We need to see if this action is inherited from MooseObjectAction. If it is, then we will
     * loop over all the Objects in MOOSE's Factory object to print them out if they have associated
     * bases matching the current task.
     */
    if (action_obj_params.have_parameter<bool>("isObjectAction") &&
        action_obj_params.get<bool>("isObjectAction"))
    {
      for (registeredMooseObjectIterator moose_obj = _factory.registeredObjectsBegin();
           moose_obj != _factory.registeredObjectsEnd();
           ++moose_obj)
      {
        InputParameters moose_obj_params = (moose_obj->second)();
        // Now that we know that this is a MooseObjectAction we need to see if it has been
        // restricted
        // in any way by the user.
        const std::vector<std::string> & buildable_types = action_obj_params.getBuildableTypes();
        std::string moose_obj_name = moose_obj->first;

        // See if the current Moose Object syntax belongs under this Action's block
        if ((buildable_types.empty() || // Not restricted
             std::find(buildable_types.begin(), buildable_types.end(), moose_obj->first) !=
                 buildable_types.end()) &&                                 // Restricted but found
            moose_obj_params.have_parameter<std::string>("_moose_base") && // Has a registered base
            _syntax.verifyMooseObjectTask(moose_obj_params.get<std::string>("_moose_base"),
                                          task) &&             // and that base is associated
            action_obj_params.mooseObjectSyntaxVisibility() && // and the Action says it's visible
            moose_obj_name.find("<JACOBIAN>") ==
                std::string::npos) // And it is not a Jacobian templated AD object
        {
          std::string name;
          size_t pos = 0;
          bool is_action_params = false;
          bool is_type = false;
          if (act_name[act_name.size() - 1] == '*')
          {
            pos = act_name.size();

            if (!action_obj_params.collapseSyntaxNesting())
              name = act_name.substr(0, pos - 1) + moose_obj_name;
            else
            {
              name = act_name.substr(0, pos - 1) + "/<type>/" + moose_obj_name;
              is_action_params = true;
            }
          }
          else
          {
            name = act_name + "/<type>/" + moose_obj_name;
            is_type = true;
          }
          moose_obj_params.set<std::string>("type") = moose_obj_name;

          auto lineinfo = _factory.getLineInfo(moose_obj_name);
          std::string classname = _factory.associatedClassName(moose_obj_name);
          name = name.substr(0, name.find("<RESIDUAL>"));
          moose_obj_name = moose_obj_name.substr(0, moose_obj_name.find("<RESIDUAL>"));
          classname = classname.substr(0, classname.find("<RESIDUAL>"));
          root.addParameters(act_name,
                             name,
                             is_type,
                             moose_obj_name,
                             is_action_params,
                             &moose_obj_params,
                             lineinfo,
                             classname);
        }
      }
    }
  }
  root.addGlobal();
}

void
Parser::buildFullTree(const std::string & search_string)
{
  std::vector<std::pair<std::string, Syntax::ActionInfo>> all_names;

  for (const auto & iter : _syntax.getAssociatedActions())
  {
    Syntax::ActionInfo act_info = iter.second;
    /**
     * If the task is nullptr that means we need to figure out which task goes with this syntax for
     * the purpose of building the Moose Object part of the tree. We will figure this out by asking
     * the ActionFactory for the registration info.
     */
    if (act_info._task == "")
      act_info._task = _action_factory.getTaskName(act_info._action);

    all_names.push_back(std::pair<std::string, Syntax::ActionInfo>(iter.first, act_info));
  }

  for (const auto & act_names : all_names)
  {
    InputParameters action_obj_params = _action_factory.getValidParams(act_names.second._action);
    _syntax_formatter->insertNode(
        act_names.first, act_names.second._action, true, &action_obj_params);

    const std::string & task = act_names.second._task;
    std::string act_name = act_names.first;

    /**
     * We need to see if this action is inherited from MooseObjectAction. If it is, then we will
     * loop over all the Objects in MOOSE's Factory object to print them out if they have associated
     * bases matching the current task.
     */
    if (action_obj_params.have_parameter<bool>("isObjectAction") &&
        action_obj_params.get<bool>("isObjectAction"))
    {
      for (registeredMooseObjectIterator moose_obj = _factory.registeredObjectsBegin();
           moose_obj != _factory.registeredObjectsEnd();
           ++moose_obj)
      {
        InputParameters moose_obj_params = (moose_obj->second)();
        /**
         * Now that we know that this is a MooseObjectAction we need to see if it has been
         * restricted in any way by the user.
         */
        const std::vector<std::string> & buildable_types = action_obj_params.getBuildableTypes();

        // See if the current Moose Object syntax belongs under this Action's block
        if ((buildable_types.empty() || // Not restricted
             std::find(buildable_types.begin(), buildable_types.end(), moose_obj->first) !=
                 buildable_types.end()) &&                                 // Restricted but found
            moose_obj_params.have_parameter<std::string>("_moose_base") && // Has a registered base
            _syntax.verifyMooseObjectTask(moose_obj_params.get<std::string>("_moose_base"),
                                          task) &&             // and that base is associated
            action_obj_params.mooseObjectSyntaxVisibility() && // and the Action says it's visible
            moose_obj->first.find("<JACOBIAN>") ==
                std::string::npos) // And it is not a Jacobian templated AD object
        {
          std::string name;
          size_t pos = 0;
          bool is_action_params = false;
          if (act_name[act_name.size() - 1] == '*')
          {
            pos = act_name.size();

            // Remove <RESIDUAL> append for AD objects
            std::string obj_name = moose_obj->first;
            removeSubstring(obj_name, "<RESIDUAL>");

            if (!action_obj_params.collapseSyntaxNesting())
              name = act_name.substr(0, pos - 1) + obj_name;
            else
            {
              name = act_name.substr(0, pos - 1) + "/<type>/" + moose_obj->first;
              is_action_params = true;
            }
          }
          else
          {
            name = act_name + "/<type>/" + moose_obj->first;
          }

          moose_obj_params.set<std::string>("type") = moose_obj->first;

          _syntax_formatter->insertNode(
              name, moose_obj->first, is_action_params, &moose_obj_params);
        }
      }
    }
  }

  // Do not change to _console, we need this printed to the stdout in all cases
  Moose::out << _syntax_formatter->print(search_string) << std::flush;
}

/**************************************************************************************************
 **************************************************************************************************
 *                                   Parameter Extraction Routines                                *
 **************************************************************************************************
 **************************************************************************************************/
using std::string;

// Template Specializations for retrieving special types from the input file
template <>
void Parser::setScalarParameter<RealVectorValue, RealVectorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealVectorValue> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<Point, Point>(const std::string & full_name,
                                              const std::string & short_name,
                                              InputParameters::Parameter<Point> * param,
                                              bool in_global,
                                              GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<RealEigenVector, RealEigenVector>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealEigenVector> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<RealEigenMatrix, RealEigenMatrix>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealEigenMatrix> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<PostprocessorName, PostprocessorName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<PostprocessorName> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<MooseEnum, MooseEnum>(const std::string & full_name,
                                                      const std::string & short_name,
                                                      InputParameters::Parameter<MooseEnum> * param,
                                                      bool in_global,
                                                      GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<MultiMooseEnum, MultiMooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<MultiMooseEnum> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<ExecFlagEnum, ExecFlagEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<ExecFlagEnum> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<RealTensorValue, RealTensorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealTensorValue> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setScalarParameter<ReporterName, std::string>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<ReporterName> * param,
    bool in_global,
    GlobalParamsAction * global_block);

// Vectors
template <>
void Parser::setVectorParameter<RealVectorValue, RealVectorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<RealVectorValue>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void
Parser::setVectorParameter<Point, Point>(const std::string & full_name,
                                         const std::string & short_name,
                                         InputParameters::Parameter<std::vector<Point>> * param,
                                         bool in_global,
                                         GlobalParamsAction * global_block);

template <>
void Parser::setVectorParameter<PostprocessorName, PostprocessorName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<PostprocessorName>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setVectorParameter<MooseEnum, MooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<MooseEnum>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setVectorParameter<VariableName, VariableName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<VariableName>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setVectorParameter<ReporterName, std::string>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<ReporterName>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Parser::setDoubleIndexParameter<Point>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<std::vector<Point>>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

void
Parser::extractParams(const std::string & prefix, InputParameters & p)
{
  std::ostringstream error_stream;
  static const std::string global_params_task = "set_global_params";
  static const std::string global_params_block_name =
      _syntax.getSyntaxByAction("GlobalParamsAction").front();

  ActionIterator act_iter = _action_wh.actionBlocksWithActionBegin(global_params_task);
  GlobalParamsAction * global_params_block = nullptr;

  // We are grabbing only the first
  if (act_iter != _action_wh.actionBlocksWithActionEnd(global_params_task))
    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);

  // Set a pointer to the current InputParameters object being parsed so that it can be referred
  // to
  // in the extraction routines
  _current_params = &p;
  _current_error_stream = &error_stream;
  for (const auto & it : p)
  {
    if (p.shouldIgnore(it.first))
      continue;

    bool found = false;
    bool in_global = false;

    for (const auto & param_name : p.paramAliases(it.first))
    {
      std::string orig_name = prefix + "/" + param_name;
      std::string full_name = orig_name;

      // Mark parameters appearing in the input file or command line
      auto node = _root->find(full_name);
      if (node && node->type() == hit::NodeType::Field)
      {
        p.inputLocation(param_name) = node->filename() + ":" + std::to_string(node->line());
        p.paramFullpath(param_name) = full_name;
        p.set_attributes(param_name, false);
        _extracted_vars.insert(
            full_name); // Keep track of all variables extracted from the input file
        found = true;
      }
      // Wait! Check the GlobalParams section
      else if (global_params_block)
      {
        full_name = global_params_block_name + "/" + param_name;
        node = _root->find(full_name);
        if (node)
        {
          p.inputLocation(param_name) = node->filename() + ":" + std::to_string(node->line());
          p.paramFullpath(param_name) = full_name;
          p.set_attributes(param_name, false);
          _extracted_vars.insert(
              full_name); // Keep track of all variables extracted from the input file
          found = true;
          in_global = true;
        }
      }
      if (found)
      {
        if (p.isPrivate(param_name) && !in_global)
          mooseError("The parameter '",
                     full_name,
                     "' is a private parameter and should not be used in an input file.");
        // avoid setting the parameter
        else if (p.isPrivate(param_name) && in_global)
          continue;

        auto & short_name = param_name;
        libMesh::Parameters::Value * par = MooseUtils::get(it.second);

#define setscalarvaltype(ptype, base, range)                                                       \
  else if (par->type() == demangle(typeid(ptype).name()))                                          \
      setScalarValueTypeParameter<ptype, range, base>(                                             \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<ptype> *>(par),                                  \
          in_global,                                                                               \
          global_params_block)
#define setscalar(ptype, base)                                                                     \
  else if (par->type() == demangle(typeid(ptype).name()))                                          \
      setScalarParameter<ptype, base>(full_name,                                                   \
                                      short_name,                                                  \
                                      dynamic_cast<InputParameters::Parameter<ptype> *>(par),      \
                                      in_global,                                                   \
                                      global_params_block)
#define setfpath(ptype)                                                                            \
  else if (par->type() == demangle(typeid(ptype).name()))                                          \
      setFilePathParam<ptype>(full_name,                                                           \
                              short_name,                                                          \
                              dynamic_cast<InputParameters::Parameter<ptype> *>(par),              \
                              p,                                                                   \
                              in_global,                                                           \
                              global_params_block)
#define setvector(ptype, base)                                                                     \
  else if (par->type() == demangle(typeid(std::vector<ptype>).name()))                             \
      setVectorParameter<ptype, base>(                                                             \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<std::vector<ptype>> *>(par),                     \
          in_global,                                                                               \
          global_params_block)
#define setmap(key_type, mapped_type)                                                              \
  else if (par->type() == demangle(typeid(std::map<key_type, mapped_type>).name()))                \
      setMapParameter(                                                                             \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<std::map<key_type, mapped_type>> *>(par),        \
          in_global,                                                                               \
          global_params_block)
#define setvectorfpath(ptype)                                                                      \
  else if (par->type() == demangle(typeid(std::vector<ptype>).name()))                             \
      setVectorFilePathParam<ptype>(                                                               \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<std::vector<ptype>> *>(par),                     \
          p,                                                                                       \
          in_global,                                                                               \
          global_params_block)
#define setvectorvector(ptype)                                                                     \
  else if (par->type() == demangle(typeid(std::vector<std::vector<ptype>>).name()))                \
      setDoubleIndexParameter<ptype>(                                                              \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<std::vector<std::vector<ptype>>> *>(par),        \
          in_global,                                                                               \
          global_params_block)
#define setvectorvectorvector(ptype)                                                               \
  else if (par->type() == demangle(typeid(std::vector<std::vector<std::vector<ptype>>>).name()))   \
      setTripleIndexParameter<ptype>(                                                              \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<                                                                            \
              InputParameters::Parameter<std::vector<std::vector<std::vector<ptype>>>> *>(par),    \
          in_global,                                                                               \
          global_params_block)

        /**
         * Scalar types
         */
        // built-ins
        // NOTE: Similar dynamic casting is done in InputParameters.C, please update appropriately
        if (false)
          ;
        setscalarvaltype(Real, double, Real);
        setscalarvaltype(int, int, long);
        setscalarvaltype(unsigned short, unsigned int, long);
        setscalarvaltype(long, int, long);
        setscalarvaltype(unsigned int, unsigned int, long);
        setscalarvaltype(unsigned long, unsigned int, long);
        setscalarvaltype(long int, int64_t, long);
        setscalarvaltype(unsigned long long, unsigned int, long);

        setscalar(bool, bool);
        setscalar(SubdomainID, int);
        setscalar(BoundaryID, int);

        // string and string-subclass types
        setscalar(string, string);
        setscalar(SubdomainName, string);
        setscalar(BoundaryName, string);
        setfpath(FileName);
        setfpath(MeshFileName);
        setfpath(FileNameNoExtension);
        setscalar(OutFileBase, string);
        setscalar(VariableName, string);
        setscalar(NonlinearVariableName, string);
        setscalar(AuxVariableName, string);
        setscalar(FunctionName, string);
        setscalar(UserObjectName, string);
        setscalar(VectorPostprocessorName, string);
        setscalar(IndicatorName, string);
        setscalar(MarkerName, string);
        setscalar(MultiAppName, string);
        setscalar(OutputName, string);
        setscalar(MaterialPropertyName, string);
        setscalar(MooseFunctorName, string);
        setscalar(MaterialName, string);
        setscalar(DistributionName, string);
        setscalar(SamplerName, string);
        setscalar(TagName, string);
        setscalar(MeshGeneratorName, string);
        setscalar(ExtraElementIDName, string);
        setscalar(PostprocessorName, PostprocessorName);
        setscalar(ExecutorName, string);
        setscalar(NonlinearSystemName, string);

        // Moose Compound Scalars
        setscalar(RealVectorValue, RealVectorValue);
        setscalar(Point, Point);
        setscalar(RealEigenVector, RealEigenVector);
        setscalar(RealEigenMatrix, RealEigenMatrix);
        setscalar(MooseEnum, MooseEnum);
        setscalar(MultiMooseEnum, MultiMooseEnum);
        setscalar(RealTensorValue, RealTensorValue);
        setscalar(ExecFlagEnum, ExecFlagEnum);
        setscalar(ReporterName, string);
        setscalar(ReporterValueName, string);
        setscalar(ParsedFunctionExpression, string);

        // vector types
        setvector(bool, bool);
        setvector(Real, double);
        setvector(int, int);
        setvector(long, int);
        setvector(unsigned int, int);

// We need to be able to parse 8-byte unsigned types when
// libmesh is configured --with-dof-id-bytes=8.  Officially,
// libmesh uses uint64_t in that scenario, which is usually
// equivalent to 'unsigned long long'.  Note that 'long long'
// has been around since C99 so most C++ compilers support it,
// but presumably uint64_t is the "most standard" way to get a
// 64-bit unsigned type, so we'll stick with that here.
#if LIBMESH_DOF_ID_BYTES == 8
        setvector(uint64_t, int);
#endif

        setvector(SubdomainID, int);
        setvector(BoundaryID, int);
        setvector(RealVectorValue, double);
        setvector(Point, Point);
        setvector(MooseEnum, MooseEnum);

        setvector(string, string);
        setvectorfpath(FileName);
        setvectorfpath(FileNameNoExtension);
        setvectorfpath(MeshFileName);
        setvector(SubdomainName, string);
        setvector(BoundaryName, string);
        setvector(NonlinearVariableName, string);
        setvector(AuxVariableName, string);
        setvector(FunctionName, string);
        setvector(UserObjectName, string);
        setvector(IndicatorName, string);
        setvector(MarkerName, string);
        setvector(MultiAppName, string);
        setvector(PostprocessorName, PostprocessorName);
        setvector(VectorPostprocessorName, string);
        setvector(OutputName, string);
        setvector(MaterialPropertyName, string);
        setvector(MooseFunctorName, string);
        setvector(MaterialName, string);
        setvector(DistributionName, string);
        setvector(SamplerName, string);
        setvector(TagName, string);
        setvector(VariableName, VariableName);
        setvector(MeshGeneratorName, string);
        setvector(ExtraElementIDName, string);
        setvector(ReporterName, string);
        setvector(ReporterValueName, string);
        setvector(ExecutorName, string);
        setvector(NonlinearSystemName, string);

        // map types
        setmap(string, Real);
        setmap(string, string);
        setmap(unsigned int, unsigned int);
        setmap(unsigned long, unsigned int);
        setmap(unsigned long long, unsigned int);

        // Double indexed types
        setvectorvector(Real);
        setvectorvector(int);
        setvectorvector(long);
        setvectorvector(unsigned int);
        setvectorvector(unsigned long long);

// See vector type explanation
#if LIBMESH_DOF_ID_BYTES == 8
        setvectorvector(uint64_t);
#endif

        setvectorvector(SubdomainID);
        setvectorvector(BoundaryID);
        setvectorvector(Point);
        setvectorvector(string);
        setvectorvector(FileName);
        setvectorvector(FileNameNoExtension);
        setvectorvector(MeshFileName);
        setvectorvector(SubdomainName);
        setvectorvector(BoundaryName);
        setvectorvector(VariableName);
        setvectorvector(NonlinearVariableName);
        setvectorvector(AuxVariableName);
        setvectorvector(FunctionName);
        setvectorvector(UserObjectName);
        setvectorvector(IndicatorName);
        setvectorvector(MarkerName);
        setvectorvector(MultiAppName);
        setvectorvector(PostprocessorName);
        setvectorvector(VectorPostprocessorName);
        setvectorvector(MarkerName);
        setvectorvector(OutputName);
        setvectorvector(MaterialPropertyName);
        setvectorvector(MooseFunctorName);
        setvectorvector(MaterialName);
        setvectorvector(DistributionName);
        setvectorvector(SamplerName);
        setvectorvector(TagName);

        // Triple indexed types
        setvectorvectorvector(Real);
        setvectorvectorvector(int);
        setvectorvectorvector(long);
        setvectorvectorvector(unsigned int);
        setvectorvectorvector(unsigned long long);

// See vector type explanation
#if LIBMESH_DOF_ID_BYTES == 8
        setvectorvectorvector(uint64_t);
#endif

        setvectorvectorvector(SubdomainID);
        setvectorvectorvector(BoundaryID);
        setvectorvectorvector(string);
        setvectorvectorvector(FileName);
        setvectorvectorvector(FileNameNoExtension);
        setvectorvectorvector(MeshFileName);
        setvectorvectorvector(SubdomainName);
        setvectorvectorvector(BoundaryName);
        setvectorvectorvector(VariableName);
        setvectorvectorvector(NonlinearVariableName);
        setvectorvectorvector(AuxVariableName);
        setvectorvectorvector(FunctionName);
        setvectorvectorvector(UserObjectName);
        setvectorvectorvector(IndicatorName);
        setvectorvectorvector(MarkerName);
        setvectorvectorvector(MultiAppName);
        setvectorvectorvector(PostprocessorName);
        setvectorvectorvector(VectorPostprocessorName);
        setvectorvectorvector(MarkerName);
        setvectorvectorvector(OutputName);
        setvectorvectorvector(MaterialPropertyName);
        setvectorvectorvector(MooseFunctorName);
        setvectorvectorvector(MaterialName);
        setvectorvectorvector(DistributionName);
        setvectorvectorvector(SamplerName);
        else
        {
          mooseError("unsupported type '", par->type(), "' for input parameter '", full_name, "'");
        }

#undef setscalarValueType
#undef setscalar
#undef setvector
#undef setvectorvectorvector
#undef setvectorvector
#undef setmap
        break;
      }
    }

    if (!found)
    {
      /**
       * Special case handling
       *   if the parameter wasn't found in the input file or the cli object the logic in this
       * branch will execute
       */

      // In the case where we have OutFileName but it wasn't actually found in the input filename,
      // we will populate it with the actual parsed filename which is available here in the
      // parser.

      InputParameters::Parameter<OutFileBase> * scalar_p =
          dynamic_cast<InputParameters::Parameter<OutFileBase> *>(MooseUtils::get(it.second));
      if (scalar_p)
      {
        std::string input_file_name = getPrimaryFileName();
        mooseAssert(input_file_name != "", "Input Filename is nullptr");
        size_t pos = input_file_name.find_last_of('.');
        mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");
        scalar_p->set() = input_file_name.substr(0, pos) + "_out";
        p.set_attributes(it.first, false);
      }
    }
  }

  // All of the parameters for this object have been extracted.  See if there are any errors
  if (!error_stream.str().empty())
    mooseError(_errmsg + error_stream.str());

  // Here we will see if there are any auto build vectors that need to be created
  std::map<std::string, std::pair<std::string, std::string>> auto_build_vectors =
      p.getAutoBuildVectors();
  for (const auto & it : auto_build_vectors)
  {
    // We'll autogenerate values iff the requested vector is not valid but both the base and
    // number
    // are valid
    const std::string & base_name = it.second.first;
    const std::string & num_repeat = it.second.second;

    if (!p.isParamValid(it.first) && p.isParamValid(base_name) && p.isParamValid(num_repeat))
    {
      unsigned int vec_size = p.get<unsigned int>(num_repeat);
      const std::string & name = p.get<std::string>(base_name);

      std::vector<VariableName> variable_names(vec_size);
      for (unsigned int i = 0; i < vec_size; ++i)
      {
        std::ostringstream oss;
        oss << name << i;
        variable_names[i] = oss.str();
      }

      // Finally set the autogenerated vector into the InputParameters object
      p.set<std::vector<VariableName>>(it.first) = variable_names;
    }
  }
}

template <typename T>
bool
toBool(const std::string & /*s*/, T & /*val*/)
{
  return false;
}

template <>
bool
toBool<bool>(const std::string & s, bool & val)
{
  return hit::toBool(s, &val);
}

template <typename T, typename Base>
void
Parser::setScalarParameter(const std::string & full_name,
                           const std::string & short_name,
                           InputParameters::Parameter<T> * param,
                           bool in_global,
                           GlobalParamsAction * global_block)
{
  try
  {
    param->set() = _root->param<Base>(full_name);
  }
  catch (hit::Error & err)
  {
    auto strval = _root->param<std::string>(full_name);

    // handle the case where the user put a number inside quotes
    auto & t = typeid(T);
    if (t == typeid(int) || t == typeid(unsigned int) || t == typeid(SubdomainID) ||
        t == typeid(BoundaryID) || t == typeid(double))
    {
      try
      {
        param->set() = MooseUtils::convert<T>(strval, true);
      }
      catch (std::invalid_argument & /*e*/)
      {
        const std::string format_type = (t == typeid(double)) ? "float" : "integer";
        _errmsg += hit::errormsg(_root->find(full_name),
                                 "invalid ",
                                 format_type,
                                 " syntax for parameter: ",
                                 full_name,
                                 "=",
                                 strval) +
                   "\n";
      }
    }
    else if (t == typeid(bool))
    {
      bool isbool = toBool(strval, param->set());
      if (!isbool)
        _errmsg += hit::errormsg(_root->find(full_name),
                                 "invalid boolean syntax for parameter: ",
                                 full_name,
                                 "=",
                                 strval) +
                   "\n";
    }
    else
      throw;
  }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = param->get();
  }
}

template <typename T>
void
Parser::setFilePathParam(const std::string & full_name,
                         const std::string & short_name,
                         InputParameters::Parameter<T> * param,
                         InputParameters & params,
                         bool in_global,
                         GlobalParamsAction * global_block)
{
  std::string prefix;
  std::string postfix = _root->param<std::string>(full_name);
  auto input_filename = getPrimaryFileName(false);
  size_t pos = input_filename.find_last_of('/');
  if (pos != std::string::npos && postfix[0] != '/' && !postfix.empty())
    prefix = input_filename.substr(0, pos + 1);

  params.rawParamVal(short_name) = postfix;
  param->set() = prefix + postfix;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = param->get();
  }
}

template <typename T, typename UP_T, typename Base>
void
Parser::setScalarValueTypeParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<T> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block)
{
  setScalarParameter<T, Base>(full_name, short_name, param, in_global, global_block);

  // If this is a range checked param, we need to make sure that the value falls within the
  // requested range
  mooseAssert(_current_params, "Current params is nullptr");

  _current_params->rangeCheck<T, UP_T>(full_name, short_name, param, *_current_error_stream);
}

template <typename T, typename Base>
void
Parser::setVectorParameter(const std::string & full_name,
                           const std::string & short_name,
                           InputParameters::Parameter<std::vector<T>> * param,
                           bool in_global,
                           GlobalParamsAction * global_block)
{
  std::vector<T> vec;
  if (_root->find(full_name))
  {
    try
    {
      auto tmp = _root->param<std::vector<Base>>(full_name);
      for (auto val : tmp)
        vec.push_back(val);
    }
    catch (hit::Error & err)
    {
      _errmsg += hit::errormsg(_root->find(full_name), err.what()) + "\n";
      return;
    }
  }

  param->set() = vec;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(param->get().size());
    for (unsigned int i = 0; i < vec.size(); ++i)
      global_block->setVectorParam<T>(short_name)[i] = param->get()[i];
  }
}

template <typename KeyType, typename MappedType>
void
Parser::setMapParameter(const std::string & full_name,
                        const std::string & short_name,
                        InputParameters::Parameter<std::map<KeyType, MappedType>> * param,
                        bool in_global,
                        GlobalParamsAction * global_block)
{
  std::map<KeyType, MappedType> the_map;
  if (_root->find(full_name))
  {
    try
    {
      const auto & string_vec = _root->param<std::vector<std::string>>(full_name);
      auto it = string_vec.begin();
      while (it != string_vec.end())
      {
        const auto & string_key = *it;
        ++it;
        if (it == string_vec.end())
        {
          _errmsg +=
              hit::errormsg(_root->find(full_name),
                            "odd number of entries in string vector for map parameter: ",
                            full_name,
                            ". There must be "
                            "an even number or else you will end up with a key without a value!") +
              "\n";
          return;
        }
        const auto & string_value = *it;
        ++it;

        std::pair<KeyType, MappedType> pr;
        // key
        try
        {
          pr.first = MooseUtils::convert<KeyType>(string_key, true);
        }
        catch (std::invalid_argument & /*e*/)
        {
          _errmsg += hit::errormsg(_root->find(full_name),
                                   "invalid ",
                                   demangle(typeid(KeyType).name()),
                                   " syntax for map parameter ",
                                   full_name,
                                   " key: ",
                                   string_key) +
                     "\n";
          return;
        }
        // value
        try
        {
          pr.second = MooseUtils::convert<MappedType>(string_value, true);
        }
        catch (std::invalid_argument & /*e*/)
        {
          _errmsg += hit::errormsg(_root->find(full_name),
                                   "invalid ",
                                   demangle(typeid(MappedType).name()),
                                   " syntax for map parameter ",
                                   full_name,
                                   " value: ",
                                   string_value) +
                     "\n";
          return;
        }

        auto insert_pr = the_map.insert(std::move(pr));
        if (!insert_pr.second)
        {
          _errmsg += hit::errormsg(_root->find(full_name),
                                   "Duplicate map entry for map parameter: ",
                                   full_name,
                                   ". The key ",
                                   string_key,
                                   " appears multiple times.") +
                     "\n";
          return;
        }
      }
    }
    catch (hit::Error & err)
    {
      _errmsg += hit::errormsg(_root->find(full_name), err.what()) + "\n";
      return;
    }
  }

  param->set() = the_map;

  if (in_global)
  {
    global_block->remove(short_name);
    auto & global_map = global_block->setParam<std::map<KeyType, MappedType>>(short_name);
    for (const auto & pair : the_map)
      global_map.insert(pair);
  }
}

template <typename T>
void
Parser::setVectorFilePathParam(const std::string & full_name,
                               const std::string & short_name,
                               InputParameters::Parameter<std::vector<T>> * param,
                               InputParameters & params,
                               bool in_global,
                               GlobalParamsAction * global_block)
{
  std::vector<T> vec;
  std::vector<std::string> rawvec;
  auto input_filename = getPrimaryFileName(false);
  if (_root->find(full_name))
  {
    auto tmp = _root->param<std::vector<std::string>>(full_name);
    params.rawParamVal(short_name) = _root->param<std::string>(full_name);
    for (auto val : tmp)
    {
      std::string prefix;
      std::string postfix = val;
      size_t pos = input_filename.find_last_of('/');
      if (pos != std::string::npos && postfix[0] != '/')
        prefix = input_filename.substr(0, pos + 1);
      rawvec.push_back(postfix);
      vec.push_back(prefix + postfix);
    }
  }

  param->set() = vec;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(param->get().size());
    for (unsigned int i = 0; i < vec.size(); ++i)
      global_block->setVectorParam<T>(short_name)[i] = param->get()[i];
  }
}

template <typename T>
void
Parser::setDoubleIndexParameter(const std::string & full_name,
                                const std::string & short_name,
                                InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                                bool in_global,
                                GlobalParamsAction * global_block)
{
  // Get the full string assigned to the variable full_name
  std::string buffer = _root->param<std::string>(full_name);

  // split vector at delim ;
  // NOTE: the substrings are _not_ of type T yet
  std::vector<std::string> first_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, ";");
  param->set().resize(first_tokenized_vector.size());

  for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
    if (!MooseUtils::tokenizeAndConvert<T>(first_tokenized_vector[j], param->set()[j]))
    {
      _errmsg +=
          hit::errormsg(_root->find(full_name), "invalid format for parameter ", full_name) + "\n";
      return;
    }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setDoubleIndexParam<T>(short_name).resize(first_tokenized_vector.size());
    for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
    {
      global_block->setDoubleIndexParam<T>(short_name)[j].resize(param->get()[j].size());
      for (unsigned int i = 0; i < param->get()[j].size(); ++i)
        global_block->setDoubleIndexParam<T>(short_name)[j][i] = param->get()[j][i];
    }
  }
}

template <typename T>
void
Parser::setTripleIndexParameter(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<std::vector<std::vector<T>>>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  // Get the full string assigned to the variable full_name
  const std::string buffer_raw = _root->param<std::string>(full_name);
  // In case the parameter is empty
  if (buffer_raw.find_first_not_of(' ', 0) == std::string::npos)
    return;

  // Add a space between neighboring delim's, before the first delim if nothing is ahead of it, and
  // after the last delim if nothing is behind it.
  std::string buffer;
  buffer.push_back(buffer_raw[0]);
  if (buffer[0] == '|' || buffer[0] == ';')
    buffer = ' ' + buffer;
  for (std::string::size_type i = 1; i < buffer_raw.size(); i++)
  {
    if ((buffer_raw[i - 1] == '|' || buffer_raw[i - 1] == ';') &&
        (buffer_raw[i] == '|' || buffer_raw[i] == ';'))
      buffer.push_back(' ');
    buffer.push_back(buffer_raw[i]);
  }
  if (buffer.back() == '|' || buffer.back() == ';')
    buffer.push_back(' ');

  // split vector at delim | to get a series of 2D subvectors
  std::vector<std::string> first_tokenized_vector;
  std::vector<std::vector<std::string>> second_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, "|");
  param->set().resize(first_tokenized_vector.size());
  second_tokenized_vector.resize(first_tokenized_vector.size());
  for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
  {
    // Identify empty subvector first
    if (first_tokenized_vector[j].find_first_not_of(' ', 0) == std::string::npos)
    {
      param->set()[j].resize(0);
      continue;
    }
    // split each 2D subvector at delim ; to get 1D sub-subvectors
    // NOTE: the 1D sub-subvectors are _not_ of type T yet
    MooseUtils::tokenize(first_tokenized_vector[j], second_tokenized_vector[j], 1, ";");
    param->set()[j].resize(second_tokenized_vector[j].size());
    for (unsigned k = 0; k < second_tokenized_vector[j].size(); ++k)
      if (!MooseUtils::tokenizeAndConvert<T>(second_tokenized_vector[j][k], param->set()[j][k]))
      {
        _errmsg +=
            hit::errormsg(_root->find(full_name), "invalid format for parameter ", full_name) +
            "\n";
        return;
      }
  }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setTripleIndexParam<T>(short_name).resize(first_tokenized_vector.size());
    for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
    {
      global_block->setTripleIndexParam<T>(short_name)[j].resize(second_tokenized_vector[j].size());
      for (unsigned k = 0; k < second_tokenized_vector[j].size(); ++k)
      {
        global_block->setTripleIndexParam<T>(short_name)[j][k].resize(param->get()[j][k].size());
        for (unsigned int i = 0; i < param->get()[j][k].size(); ++i)
          global_block->setTripleIndexParam<T>(short_name)[j][k][i] = param->get()[j][k][i];
      }
    }
  }
}

template <typename T>
void
Parser::setScalarComponentParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<T> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block)
{
  std::vector<double> vec;
  try
  {
    vec = _root->param<std::vector<double>>(full_name);
  }
  catch (hit::Error & err)
  {
    _errmsg += hit::errormsg(_root->find(full_name), err.what()) + "\n";
    return;
  }

  if (vec.size() != LIBMESH_DIM)
  {
    _errmsg += hit::errormsg(_root->find(full_name),
                             "wrong number of values in scalar component parameter ",
                             full_name,
                             ": size ",
                             vec.size(),
                             " is not a multiple of ",
                             LIBMESH_DIM) +
               "\n";
    return;
  }

  T value;
  for (unsigned int i = 0; i < vec.size(); ++i)
    value(i) = Real(vec[i]);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = value;
  }
}

template <typename T>
void
Parser::setVectorComponentParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<std::vector<T>> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block)
{
  std::vector<double> vec;
  try
  {
    vec = _root->param<std::vector<double>>(full_name);
  }
  catch (hit::Error & err)
  {
    _errmsg += hit::errormsg(_root->find(full_name), err.what()) + "\n";
    return;
  }

  if (vec.size() % LIBMESH_DIM)
  {
    _errmsg += hit::errormsg(_root->find(full_name),
                             "wrong number of values in vector component parameter ",
                             full_name,
                             ": size ",
                             vec.size(),
                             " is not a multiple of ",
                             LIBMESH_DIM) +
               "\n";
    return;
  }

  std::vector<T> values;
  for (unsigned int i = 0; i < vec.size() / LIBMESH_DIM; ++i)
  {
    T value;
    for (int j = 0; j < LIBMESH_DIM; ++j)
      value(j) = Real(vec[i * LIBMESH_DIM + j]);
    values.push_back(value);
  }

  param->set() = values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(vec.size(), values[0]);
    for (unsigned int i = 0; i < vec.size() / LIBMESH_DIM; ++i)
      global_block->setVectorParam<T>(short_name)[i] = values[0];
  }
}

template <typename T>
void
Parser::setVectorVectorComponentParameter(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<std::vector<T>>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  // Get the full string assigned to the variable full_name
  std::string buffer = _root->param<std::string>(full_name);

  // split vector at delim ;
  // NOTE: the substrings are _not_ of type T yet
  std::vector<std::string> first_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, ";");
  param->set().resize(first_tokenized_vector.size());

  // get a vector<vector<double>> first
  std::vector<std::vector<double>> vecvec(first_tokenized_vector.size());
  for (unsigned j = 0; j < vecvec.size(); ++j)
    if (!MooseUtils::tokenizeAndConvert<double>(first_tokenized_vector[j], vecvec[j]))
    {
      _errmsg +=
          hit::errormsg(_root->find(full_name), "invalid format for parameter ", full_name) + "\n";
      return;
    }

  for (const auto & vec : vecvec)
    if (vec.size() % LIBMESH_DIM)
    {
      _errmsg +=
          hit::errormsg(_root->find(full_name),
                        "wrong number of values in double-indexed vector component parameter ",
                        full_name,
                        ": size of subcomponent ",
                        vec.size(),
                        " is not a multiple of ",
                        LIBMESH_DIM) +
          "\n";
      return;
    }

  // convert vector<vector<double>> to vector<vector<T>>
  std::vector<std::vector<T>> values(vecvec.size());
  for (unsigned int id_vec = 0; id_vec < vecvec.size(); ++id_vec)
    for (unsigned int i = 0; i < vecvec[id_vec].size() / LIBMESH_DIM; ++i)
    {
      T value;
      for (int j = 0; j < LIBMESH_DIM; ++j)
        value(j) = Real(vecvec[id_vec][i * LIBMESH_DIM + j]);
      values[id_vec].push_back(value);
    }

  param->set() = values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setDoubleIndexParam<T>(short_name).resize(vecvec.size());
    for (unsigned j = 0; j < vecvec.size(); ++j)
    {
      global_block->setDoubleIndexParam<T>(short_name)[j].resize(param->get()[j].size() /
                                                                 LIBMESH_DIM);
      for (unsigned int i = 0; i < param->get()[j].size() / LIBMESH_DIM; ++i)
        global_block->setDoubleIndexParam<T>(short_name)[j][i] = values[j][i];
    }
  }
}

template <>
void
Parser::setScalarParameter<RealVectorValue, RealVectorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealVectorValue> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  setScalarComponentParameter(full_name, short_name, param, in_global, global_block);
}

template <>
void
Parser::setScalarParameter<Point, Point>(const std::string & full_name,
                                         const std::string & short_name,
                                         InputParameters::Parameter<Point> * param,
                                         bool in_global,
                                         GlobalParamsAction * global_block)
{
  setScalarComponentParameter(full_name, short_name, param, in_global, global_block);
}

template <>
void
Parser::setScalarParameter<RealEigenVector, RealEigenVector>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealEigenVector> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  std::vector<double> vec;
  try
  {
    vec = _root->param<std::vector<double>>(full_name);
  }
  catch (hit::Error & err)
  {
    _errmsg += hit::errormsg(_root->find(full_name), err.what()) + "\n";
    return;
  }

  RealEigenVector value(vec.size());
  for (unsigned int i = 0; i < vec.size(); ++i)
    value(i) = Real(vec[i]);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<RealEigenVector>(short_name) = value;
  }
}

template <>
void
Parser::setScalarParameter<RealEigenMatrix, RealEigenMatrix>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealEigenMatrix> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  // Get the full string assigned to the variable full_name
  std::string buffer = _root->param<std::string>(full_name);

  // split vector at delim ;
  // NOTE: the substrings are _not_ of type T yet
  std::vector<std::string> first_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, ";");

  std::vector<std::vector<Real>> values(first_tokenized_vector.size());

  for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
  {
    if (!MooseUtils::tokenizeAndConvert<Real>(first_tokenized_vector[j], values[j]))
    {
      _errmsg +=
          hit::errormsg(_root->find(full_name), "invalid format for parameter ", full_name) + "\n";
      return;
    }
    if (j != 0 && values[j].size() != values[0].size())
    {
      _errmsg +=
          hit::errormsg(_root->find(full_name), "invalid format for parameter ", full_name) + "\n";
      return;
    }
  }

  RealEigenMatrix value(values.size(), values[0].size());
  for (unsigned int i = 0; i < values.size(); ++i)
    for (unsigned int j = 0; j < values[i].size(); ++j)
      value(i, j) = values[i][j];

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<RealEigenMatrix>(short_name) = value;
  }
}

template <>
void
Parser::setScalarParameter<MooseEnum, MooseEnum>(const std::string & full_name,
                                                 const std::string & short_name,
                                                 InputParameters::Parameter<MooseEnum> * param,
                                                 bool in_global,
                                                 GlobalParamsAction * global_block)
{
  MooseEnum current_param = param->get();

  std::string value = _root->param<std::string>(full_name);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<MooseEnum>(short_name) = current_param;
  }
}

template <>
void
Parser::setScalarParameter<MultiMooseEnum, MultiMooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<MultiMooseEnum> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  MultiMooseEnum current_param = param->get();

  auto vec = _root->param<std::vector<std::string>>(full_name);

  std::string raw_values;
  for (unsigned int i = 0; i < vec.size(); ++i)
    raw_values += ' ' + vec[i];

  param->set() = raw_values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<MultiMooseEnum>(short_name) = current_param;
  }
}

template <>
void
Parser::setScalarParameter<ExecFlagEnum, ExecFlagEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<ExecFlagEnum> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  ExecFlagEnum current_param = param->get();
  auto vec = _root->param<std::vector<std::string>>(full_name);

  std::string raw_values;
  for (unsigned int i = 0; i < vec.size(); ++i)
    raw_values += ' ' + vec[i];

  param->set() = raw_values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<ExecFlagEnum>(short_name) = current_param;
  }
}

template <>
void
Parser::setScalarParameter<RealTensorValue, RealTensorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealTensorValue> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  auto vec = _root->param<std::vector<double>>(full_name);
  if (vec.size() != LIBMESH_DIM * LIBMESH_DIM)
  {
    _errmsg += hit::errormsg(_root->find(full_name),
                             "invalid RealTensorValue parameter ",
                             full_name,
                             ": size is ",
                             vec.size(),
                             " but should be ",
                             LIBMESH_DIM * LIBMESH_DIM) +
               "\n";
    return;
  }

  RealTensorValue value;
  for (int i = 0; i < LIBMESH_DIM; ++i)
    for (int j = 0; j < LIBMESH_DIM; ++j)
      value(i, j) = Real(vec[i * LIBMESH_DIM + j]);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<RealTensorValue>(short_name) = value;
  }
}

// Specialization for coupling a Real value where a postprocessor would be needed in MOOSE
template <>
void
Parser::setScalarParameter<PostprocessorName, PostprocessorName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<PostprocessorName> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  PostprocessorName pps_name = _root->param<std::string>(full_name);
  param->set() = pps_name;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<PostprocessorName>(short_name) = pps_name;
  }
}

template <>
void
Parser::setScalarParameter<ReporterName, std::string>(
    const std::string & full_name,
    const std::string & /*short_name*/,
    InputParameters::Parameter<ReporterName> * param,
    bool /*in_global*/,
    GlobalParamsAction * /*global_block*/)
{
  std::vector<std::string> names = MooseUtils::rsplit(_root->param<std::string>(full_name), "/", 2);
  if (names.size() != 2)
    _errmsg += hit::errormsg(_root->find(full_name),
                             "The supplied name ReporterName '",
                             full_name,
                             "' must contain the '/' delimiter.");
  else
    param->set() = ReporterName(names[0], names[1]);
}

template <>
void
Parser::setVectorParameter<RealVectorValue, RealVectorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<RealVectorValue>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  setVectorComponentParameter(full_name, short_name, param, in_global, global_block);
}

template <>
void
Parser::setVectorParameter<Point, Point>(const std::string & full_name,
                                         const std::string & short_name,
                                         InputParameters::Parameter<std::vector<Point>> * param,
                                         bool in_global,
                                         GlobalParamsAction * global_block)
{
  setVectorComponentParameter(full_name, short_name, param, in_global, global_block);
}

template <>
void
Parser::setVectorParameter<MooseEnum, MooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<MooseEnum>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  std::vector<MooseEnum> enum_values = param->get();
  std::vector<std::string> values(enum_values.size());
  for (unsigned int i = 0; i < values.size(); ++i)
    values[i] = static_cast<std::string>(enum_values[i]);

  /**
   * With MOOSE Enums we need a default object so it should have been passed in the param pointer.
   * We are only going to use the first item in the vector (values[0]) and ignore the rest.
   */
  std::vector<std::string> vec;
  if (_root->find(full_name))
  {
    vec = _root->param<std::vector<std::string>>(full_name);
    param->set().resize(vec.size(), enum_values[0]);
  }

  for (unsigned int i = 0; i < vec.size(); ++i)
    param->set()[i] = vec[i];

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<MooseEnum>(short_name).resize(vec.size(), enum_values[0]);
    for (unsigned int i = 0; i < vec.size(); ++i)
      global_block->setVectorParam<MooseEnum>(short_name)[i] = values[0];
  }
}

template <>
void
Parser::setVectorParameter<PostprocessorName, PostprocessorName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<PostprocessorName>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  std::vector<std::string> pps_names = _root->param<std::vector<std::string>>(full_name);
  unsigned int n = pps_names.size();
  param->set().resize(n);

  for (unsigned int j = 0; j < n; ++j)
    param->set()[j] = pps_names[j];

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<PostprocessorName>(short_name).resize(n, "");
    for (unsigned int j = 0; j < n; ++j)
      global_block->setVectorParam<PostprocessorName>(short_name)[j] = pps_names[j];
  }
}

/**
 * Specialization for coupling vectors. This routine handles default values and auto generated
 * VariableValue vectors.
 */
template <>
void
Parser::setVectorParameter<VariableName, VariableName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<VariableName>> * param,
    bool /*in_global*/,
    GlobalParamsAction * /*global_block*/)
{
  auto vec = _root->param<std::vector<std::string>>(full_name);
  auto strval = _root->param<std::string>(full_name);
  std::vector<VariableName> var_names(vec.size());

  bool has_var_names = false;
  for (unsigned int i = 0; i < vec.size(); ++i)
  {
    VariableName var_name = vec[i];

    Real real_value;
    std::istringstream ss(var_name);

    // If we are able to convert this value into a Real, then set a default coupled value
    // NOTE: parameter must be either all default or no defaults
    if (ss >> real_value && ss.eof())
      _current_params->defaultCoupledValue(short_name, real_value, i);
    else
    {
      var_names[i] = var_name;
      has_var_names = true;
    }
  }

  if (has_var_names)
  {
    param->set().resize(vec.size());

    for (unsigned int i = 0; i < vec.size(); ++i)
      if (var_names[i] == "")
      {
        _errmsg +=
            hit::errormsg(
                _root->find(full_name),
                "invalid value for ",
                full_name,
                ":\n"
                "    MOOSE does not currently support a coupled vector where some parameters are ",
                "reals and others are variables") +
            "\n";
        return;
      }
      else
        param->set()[i] = var_names[i];
  }
}

template <>
void
Parser::setVectorParameter<ReporterName, std::string>(
    const std::string & full_name,
    const std::string & /*short_name*/,
    InputParameters::Parameter<std::vector<ReporterName>> * param,
    bool /*in_global*/,
    GlobalParamsAction * /*global_block*/)
{
  auto rnames = _root->param<std::vector<std::string>>(full_name);
  param->set().resize(rnames.size());

  for (unsigned int i = 0; i < rnames.size(); ++i)
  {
    std::vector<std::string> names = MooseUtils::rsplit(rnames[i], "/", 2);
    if (names.size() != 2)
      _errmsg += hit::errormsg(_root->find(full_name),
                               "The supplied name ReporterName '",
                               rnames[i],
                               "' must contain the '/' delimiter.");
    else
      param->set()[i] = ReporterName(names[0], names[1]);
  }
}

template <>
void
Parser::setDoubleIndexParameter<Point>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<std::vector<Point>>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  setVectorVectorComponentParameter(full_name, short_name, param, in_global, global_block);
}
