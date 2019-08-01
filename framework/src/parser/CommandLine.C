//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CommandLine.h"
#include "MooseInit.h"
#include "MooseUtils.h"
#include "InputParameters.h"

// Contrib RE
#include "pcrecpp.h"

// C++ includes
#include <iomanip>

CommandLine::CommandLine(int argc, char * argv[]) { addArguments(argc, argv); }

CommandLine::CommandLine(const CommandLine & other)
  : _cli_options(other._cli_options),
    _used_hiti(other._used_hiti),
    _hiti(other._hiti),
    _argv(other._argv),
    _args(other._args)
{
}

void
CommandLine::addArguments(int argc, char * argv[])
{
  for (int i = 0; i < argc; i++)
    addArgument(argv[i]);
}

void
CommandLine::addArgument(std::string arg)
{
  _argv.push_back(arg);

  auto arg_value = std::string(arg);

  // Handle using a "="
  if (arg_value.find("=") != std::string::npos)
  {
    std::vector<std::string> arg_split;

    MooseUtils::tokenize(arg_value, arg_split, 1, "=");

    for (auto & arg_piece : arg_split)
      _args.push_back(MooseUtils::trim(arg_piece));
  }
  else
    _args.push_back(arg_value);
}

CommandLine::~CommandLine() {}

void
CommandLine::initForMultiApp(const std::string & subapp_full_name)
{
  // Get the name and number for the current sub-application
  std::string sub_name;
  int sub_num = std::numeric_limits<int>::min(); // this initial value should never be used
  pcrecpp::RE("(\\S*?)(\\d*)").FullMatch(subapp_full_name, &sub_name, &sub_num);

  if (sub_num == std::numeric_limits<int>::min())
    mooseError("The sub-application name '", subapp_full_name, "' must contain a number.");

  // "remove" CLI args for other sub-applications; remove_if just moves items to the end, so
  // an erase is needed to actually remove the items
  auto new_end =
      std::remove_if(_argv.begin(), _argv.end(), [&sub_name, sub_num](const std::string & arg) {
        return hitArgCmp(arg, sub_name, sub_num) == HitCmpResult::WRONG;
      });
  _argv.erase(new_end, _argv.end());

  // Clear hit CLI arguments, these will be populated after the sub-application is created
  _hiti.clear();
  _used_hiti.clear();
}

void
CommandLine::addCommandLineOptionsFromParams(InputParameters & params)
{
  for (const auto & it : params)
  {
    Option cli_opt;
    std::vector<std::string> syntax;
    std::string orig_name = it.first;

    cli_opt.description = params.getDocString(orig_name);
    if (!params.isPrivate(orig_name))
      // If a param is private then it shouldn't have any command line syntax.
      syntax = params.getSyntax(orig_name);
    cli_opt.cli_syntax = syntax;
    cli_opt.required = false;
    InputParameters::Parameter<bool> * bool_type =
        dynamic_cast<InputParameters::Parameter<bool> *>(it.second);
    if (bool_type)
      cli_opt.argument_type = CommandLine::NONE;
    else
      cli_opt.argument_type = CommandLine::REQUIRED;

    addOption(orig_name, cli_opt);
  }
}

void
CommandLine::populateInputParams(InputParameters & params)
{
  for (const auto & it : params)
  {
    std::string orig_name = it.first;

    if (search(orig_name))
    {
      {
        InputParameters::Parameter<std::string> * string_type =
            dynamic_cast<InputParameters::Parameter<std::string> *>(it.second);
        if (string_type)
        {
          search(orig_name, params.set<std::string>(orig_name));
          continue;
        }

        InputParameters::Parameter<Real> * real_type =
            dynamic_cast<InputParameters::Parameter<Real> *>(it.second);
        if (real_type)
        {
          search(orig_name, params.set<Real>(orig_name));
          continue;
        }

        InputParameters::Parameter<unsigned int> * uint_type =
            dynamic_cast<InputParameters::Parameter<unsigned int> *>(it.second);
        if (uint_type)
        {
          search(orig_name, params.set<unsigned int>(orig_name));
          continue;
        }

        InputParameters::Parameter<int> * int_type =
            dynamic_cast<InputParameters::Parameter<int> *>(it.second);
        if (int_type)
        {
          search(orig_name, params.set<int>(orig_name));
          continue;
        }

        InputParameters::Parameter<bool> * bool_type =
            dynamic_cast<InputParameters::Parameter<bool> *>(it.second);
        if (bool_type)
        {
          search(orig_name, params.set<bool>(orig_name));
          continue;
        }
      }
    }
    else if (params.isParamRequired(orig_name))
      mooseError("Missing required command-line parameter: ",
                 orig_name,
                 "\nDoc String: ",
                 params.getDocString(orig_name));
  }
}

void
CommandLine::addOption(const std::string & name, Option cli_opt)
{
  for (const auto & stx : cli_opt.cli_syntax)
    cli_opt.cli_switch.push_back(stx.substr(0, stx.find_first_of(" =")));

  _cli_options[name] = cli_opt;
}

bool
CommandLine::search(const std::string & option_name)
{
  std::map<std::string, Option>::iterator pos = _cli_options.find(option_name);
  if (pos != _cli_options.end())
  {
    for (const auto & search_string : pos->second.cli_switch)
      for (auto & arg : _args)
        if (arg == search_string)
          return true;

    if (pos->second.required)
    {
      printUsage();
      mooseError("Required parameter: ", option_name, " missing");
    }
    return false;
  }
  mooseError("Unrecognized option name: ", option_name);
}

void
CommandLine::printUsage() const
{
  // Grab the first item out of argv
  std::string command(_args[0]);
  command.substr(command.find_last_of("/\\") + 1);

  Moose::out << "Usage: " << command << " [<options>]\n\n"
             << "Options:\n"
             << std::left;

  for (const auto & i : _cli_options)
  {
    if (i.second.cli_syntax.empty())
      continue;

    std::stringstream oss;
    for (unsigned int j = 0; j < i.second.cli_syntax.size(); ++j)
    {
      if (j)
        oss << " ";
      oss << i.second.cli_syntax[j];
    }
    Moose::out << "  " << std::setw(50) << oss.str() << i.second.description << "\n";
  }

  Moose::out << "\nSolver Options:\n"
             << "  See solver manual for details (Petsc or Trilinos)\n";
}

template <>
void
CommandLine::setArgument<std::string>(std::stringstream & stream, std::string & argument)
{
  argument = stream.str();
}

HitCmpResult hitArgCmp(const std::string & arg, const std::string & app_name, int app_num)
{
  int arg_num = -1;
  std::string arg_name;
  auto pos = arg.find(":", 0);
  if (pos == 0)
    return HitCmpResult::GLOBAL;
  if (pos == std::string::npos)
    return HitCmpResult::MAIN;
  pcrecpp::RE("(\\S*?)(\\d*):").PartialMatch(arg, &arg_name, &arg_num);
  if (app_name == arg_name && (arg_num == -1 || arg_num == app_num)) return HitCmpResult::MATCH;
  return HitCmpResult::WRONG;
}
