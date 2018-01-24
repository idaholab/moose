//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

// Moose Includes
#include "MooseError.h"

#include "libmesh/parallel.h"

// C++ includes
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <set>

// Forward Declaration
class InputParameters;

/**
 * This class wraps provides and tracks access to command line parameters.
 */
class CommandLine
{
public:
  /// Type of argument for a given option
  enum ARGUMENT
  {
    NONE,
    OPTIONAL,
    REQUIRED
  };

  struct Option
  {
    std::string description;
    std::vector<std::string> cli_syntax;
    bool required;
    ARGUMENT argument_type;
    /// This gets filled in automagicaly when calling addOption()
    std::vector<std::string> cli_switch;
  };

  CommandLine(int argc, char * argv[]);
  virtual ~CommandLine();

  void parseInputParams(const InputParameters & params);

  void addCommandLineOptionsFromParams(InputParameters & params);

  void populateInputParams(InputParameters & params);

  void addOption(const std::string & name, Option cli_opt);

  /**
   * This routine searches the command line for the given option "handle"
   * and returns a boolean indicating whether it was found.  If the given
   * option has an argument it is also filled in.
   */
  bool search(const std::string & option_name);

  template <typename T>
  bool search(const std::string & option_name, T & argument);

  /**
   * Print the usage info for this command line
   */
  void printUsage() const;

  int argc() { return _argc; }
  char ** argv() { return _argv; }

  // this needs to be tracked here because CommandLine has a global shared instance across all
  // multiapps/subapps - and we need to track used/unused CLI hit params globally so we know
  // which ones don't get used - this can't happen at the within-app level.
  void markHitParamUsed(int argi) { _used_hiti.insert(argi); };
  void markHitParam(int argi) { _hiti.insert(argi); }

  // Returns the unused CLI hit parameters.  This accounts for different CLI params being used
  // by different processes in a process-parallel run, so the communicator is needed to rendezvous
  // which parameters have been used between them all.
  std::set<int> unused(const Parallel::Communicator & comm)
  {
    comm.set_union(_hiti);
    comm.set_union(_used_hiti);

    std::set<int> unused;
    for (int i : _hiti)
    {
      if (_used_hiti.count(i) == 0)
        unused.insert(i);
    }
    return unused;
  }

protected:
  /// Command line options
  std::map<std::string, Option> _cli_options;

private:
  std::set<int> _used_hiti;
  std::set<int> _hiti;
  int _argc = 0;
  char ** _argv = nullptr;
  std::vector<std::string> _args;
};

template <typename T>
bool
CommandLine::search(const std::string & option_name, T & argument)
{
  std::map<std::string, Option>::iterator pos = _cli_options.find(option_name);
  if (pos != _cli_options.end())
  {
    for (unsigned int i = 0; i < pos->second.cli_switch.size(); ++i)
    {

      for (size_t j = 0; j < _args.size(); j++)
      {
        auto arg = _args[j];
        if (arg == pos->second.cli_switch[i])
        {
          // "Flag" CLI options are added as Boolean types, when we see them
          // we set the Boolean argument to true
          if (pos->second.argument_type == NONE)
            argument = true;
          else if (j + 1 < _args.size())
          {
            std::stringstream ss;
            ss << _args[j + 1];
            ss >> argument;
          }
          return true;
        }
      }
    }

    if (pos->second.required)
    {
      Moose::err << "Required parameter: " << option_name << " missing\n";
      printUsage();
    }
    return false;
  }
  mooseError("Unrecognized option name");
}

#endif // COMMANDLINE_H
