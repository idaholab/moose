/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/


// MOOSE includes
#include "CommandLine.h"
#include "MooseInit.h"
#include "MooseUtils.h"
#include "InputParameters.h"

// C++ includes
#include <iomanip>


CommandLine::CommandLine(int argc, char *argv[]) :
    _get_pot(libmesh_make_unique<GetPot>(argc, argv)),
    _has_prefix(false)
{
}

CommandLine::~CommandLine()
{
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
    syntax = params.getSyntax(orig_name);
    cli_opt.cli_syntax = syntax;
    cli_opt.required = false;
    InputParameters::Parameter<bool> * bool_type = dynamic_cast<InputParameters::Parameter<bool>*>(it.second);
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
        InputParameters::Parameter<std::string> * string_type = dynamic_cast<InputParameters::Parameter<std::string>*>(it.second);
        if (string_type)
        {
          search(orig_name, params.set<std::string>(orig_name));
          continue;
        }

        InputParameters::Parameter<Real> * real_type = dynamic_cast<InputParameters::Parameter<Real>*>(it.second);
        if (real_type)
        {
          search(orig_name, params.set<Real>(orig_name));
          continue;
        }

        InputParameters::Parameter<unsigned int> * uint_type = dynamic_cast<InputParameters::Parameter<unsigned int>*>(it.second);
        if (uint_type)
        {
          search(orig_name, params.set<unsigned int>(orig_name));
          continue;
        }

        InputParameters::Parameter<int> * int_type = dynamic_cast<InputParameters::Parameter<int>*>(it.second);
        if (int_type)
        {
          search(orig_name, params.set<int>(orig_name));
          continue;
        }

        InputParameters::Parameter<bool> * bool_type = dynamic_cast<InputParameters::Parameter<bool>*>(it.second);
        if (bool_type)
        {
          search(orig_name, params.set<bool>(orig_name));
          continue;
        }
      }
    }
    else if (params.isParamRequired(orig_name))
      mooseError("Missing required command-line parameter: " << orig_name << std::endl << "Doc String: " << params.getDocString(orig_name));
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
      if (_get_pot->search(search_string))
        return true;

    if (pos->second.required)
    {
      printUsage();
      mooseError("Required parameter: " << option_name << " missing");
    }
  }
  else
    mooseError("Unrecognized option name");

  return false;
}

void
CommandLine::printUsage() const
{
  // Grab the first item out of argv
  std::string command((*_get_pot)[0]);
  command.substr(command.find_last_of("/\\")+1);

  Moose::out << "Usage: " << command << " [<options>]\n\n"
             << "Options:\n" << std::left;

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

void
CommandLine::buildVarsSet()
{
  for (const char* var; (var = _get_pot->next_nominus()) != NULL; )
  {
    std::vector<std::string> name_value_pairs;
    MooseUtils::tokenize(var, name_value_pairs, 0, "=");
    _command_line_vars.insert(name_value_pairs[0]);
  }
}

bool
CommandLine::isVariableOnCommandLine(const std::string &name) const
{
  return _command_line_vars.find(name) != _command_line_vars.end();
}

bool
CommandLine::haveVariable(const std::string & name, bool allow_prefix_change)
{
  // Make sure the CommandLine object is in the right state with the right prefix
  resetPrefix();
  if (_get_pot->have_variable(name))
    return true;

  if (allow_prefix_change)
  {
    /**
     * Try falling back to the base prefix before giving up. Note that this
     * will modify the behavior of invocations to GetPot after this method
     * completes. This is desired and intended behavior. After allowing
     * the prefix to fall back, one should call resetPrefix() to restore
     * normal behavior.
     */
    if (_has_prefix)
    {
      _get_pot->set_prefix((_base_prefix + ":").c_str());
      if (_get_pot->have_variable(name))
        return true;

    /**
     * As a final attempt we'll see if the user has passed a global command line parameter
     * in the form ":name=value". Similarly to the normal subapp prefix, this will also
     * modify subsequent invocations to GetPot until resetPrefix() has been called.
     */
      _get_pot->set_prefix(":");
      if (_get_pot->have_variable(name))
        return true;

      /**
       * We failed to find the parameter with the subapp prefix (if applicable) or
       * in the global section so we need to reset the prefix back to the way it was.
       */
      resetPrefix();
    }
  }

  return false;
}

void
CommandLine::setPrefix(const std::string & name, const std::string & num)
{
  _base_prefix = name;
  _prefix_num = num;
  _has_prefix = true;

  /**
   * By default we'll append the name and num together and delimit with a colon for the GetPot parser.
   * However, we may need to fall back and check only the base prefix if a user wants to apply a parameter
   * override to all Multiapps with a given name.
   */
  _get_pot->set_prefix((name + num + ":").c_str());
}

void
CommandLine::resetPrefix()
{
  // If this CommandLine instance doesn't have a prefix, do nothing
  if (!_has_prefix)
    return;

  _get_pot->set_prefix((_base_prefix + _prefix_num + ":").c_str());
}

void
CommandLine::print(const char * prefix, std::ostream & out_stream, unsigned int skip_count)
{
  _get_pot->print(prefix, out_stream, skip_count);
}
