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

// C++ includes
#include <iomanip>

CommandLine::CommandLine(int argc, char * argv[]) : _argc(argc), _argv(argv)
{
  for (int i = 0; i < argc; i++)
    _args.push_back(std::string(argv[i]));
}

CommandLine::~CommandLine() {}

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
    {
      for (auto & arg : _args)
      {
        if (arg == search_string)
          return true;
      }
    }

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
