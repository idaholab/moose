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
  auto new_end = std::remove_if(
      _argv.begin(),
      _argv.end(),
      [&sub_name, sub_num](const std::string & arg)
      {
        // Determine if the current command line argument ('arg') and extract the sub-application
        // name and number. If 'arg' is not command line argument for sub-application then the regex
        // match fails and the argument is retained.
        std::string arg_sub_name;
        int arg_sub_num = -1;
        pcrecpp::RE("(\\S*?)(\\d*):").PartialMatch(arg, &arg_sub_name, &arg_sub_num);
        if (!arg_sub_name.empty())
        {
          // The argument should be retained if names match and the current argument doesn't have
          // a number or the supplied sub-application number and the current argument number match
          bool keep = (sub_name == arg_sub_name && (arg_sub_num == -1 || arg_sub_num == sub_num));
          return !keep;
        }
        return false;
      });
  _argv.erase(new_end, _argv.end());

  // If there is an argument meant for a nested subapp, we will want to remove
  // the leading app name
  for (auto & arg : _argv)
  {
    auto pos = arg.find(":", 0);
    if (pos != std::string::npos && arg.find(":", pos + 1) != std::string::npos)
      arg = subapp_full_name + "_" + arg.substr(pos + 1, arg.length() - pos - 1);
  }

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

    if (params.have_parameter<bool>(orig_name))
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
      if (params.have_parameter<std::string>(orig_name))
      {
        search(orig_name, params.set<std::string>(orig_name));
        continue;
      }

      if (params.have_parameter<std::vector<std::string>>(orig_name))
      {
        search(orig_name, params.set<std::vector<std::string>>(orig_name));
        continue;
      }

      if (params.have_parameter<Real>(orig_name))
      {
        search(orig_name, params.set<Real>(orig_name));
        continue;
      }

      if (params.have_parameter<unsigned int>(orig_name))
      {
        search(orig_name, params.set<unsigned int>(orig_name));
        continue;
      }

      if (params.have_parameter<int>(orig_name))
      {
        search(orig_name, params.set<int>(orig_name));
        continue;
      }

      if (params.have_parameter<bool>(orig_name))
      {
        search(orig_name, params.set<bool>(orig_name));
        continue;
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

std::vector<std::string>::const_iterator
CommandLine::find(const std::string & option_name) const
{
  auto pos = _cli_options.find(option_name);
  auto it = _args.end();

  if (pos != _cli_options.end())
  {
    for (const auto & search_string : pos->second.cli_switch)
    {
      auto it = std::find(_args.begin(), _args.end(), search_string);
      if (it != _args.end())
        return it;
    }
  }

  return it;
}

std::vector<std::string>::const_iterator
CommandLine::begin() const
{
  return _args.begin();
}

std::vector<std::string>::const_iterator
CommandLine::end() const
{
  return _args.end();
}

bool
CommandLine::search(const std::string & option_name)
{
  auto pos = _cli_options.find(option_name);
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

std::string
CommandLine::getExecutableName() const
{
  // Grab the first item out of argv
  std::string command(_args[0]);
  command.substr(command.find_last_of("/\\") + 1);
  return command;
}

void
CommandLine::printUsage() const
{
  Moose::out << "Usage: " << getExecutableName() << " [<options>]\n\n"
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
             << "  See solver manual for details (Petsc or Trilinos)" << std::endl;
}

template <>
void
CommandLine::setArgument<std::string>(std::stringstream & stream, std::string & argument)
{
  argument = stream.str();
}
