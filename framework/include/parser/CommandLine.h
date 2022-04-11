//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "MooseError.h"
#include "Conversion.h"

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
  CommandLine(const CommandLine & other);
  virtual ~CommandLine();

  void addArguments(int argc, char * argv[]);
  void addArgument(std::string);

  /**
   * Removes multiapp parameters not associated with the supplied name.
   *
   * When a sub-application is created the CommandLine object from the master application is
   * copied and supplied to the sub-app. This method cleans up the copy so it is ready to
   * be used for a sub-application by removing parameters that are not associated with the provided
   * sub-application name.
   *
   * See MultiApp::createApp
   */
  void initForMultiApp(const std::string &);

  /**
   * Return the raw argv arguments as a vector.
   */
  const std::vector<std::string> & getArguments() { return _argv; }

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

  template <typename T>
  bool search(const std::string & option_name, std::vector<T> & argument);

  /**
   * Returns an iterator to the underlying argument vector to the position of the option or
   * end if the option is not on the command line.
   */
  std::vector<std::string>::const_iterator find(const std::string & option_name) const;

  // Return an iterator to the beginning of the container of CLI options
  std::vector<std::string>::const_iterator begin() const;

  // Return an iterator to the beginning of the container of CLI options
  std::vector<std::string>::const_iterator end() const;

  /**
   * Get the executable name.
   */
  std::string getExecutableName() const;

  /**
   * Print the usage info for this command line
   */
  void printUsage() const;

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
  /**
   * Used to set the argument value, allows specialization
   */
  template <typename T>
  void setArgument(std::stringstream & stream, T & argument);

  /// Command line options
  std::map<std::string, Option> _cli_options;

private:
  /// indices of CLI args that have been marked as used
  std::set<int> _used_hiti;

  /// indices of CLI args that are HIT syntax parameters
  std::set<int> _hiti;

  /// Storage for the raw argv
  std::vector<std::string> _argv;

  std::vector<std::string> _args;
};

template <typename T>
void
CommandLine::setArgument(std::stringstream & stream, T & argument)
{
  stream >> argument;
}

// Specialization for std::string
template <>
void CommandLine::setArgument<std::string>(std::stringstream & stream, std::string & argument);

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

            setArgument(ss, argument);
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

template <typename T>
bool
CommandLine::search(const std::string & option_name, std::vector<T> & argument)
{
  std::map<std::string, Option>::iterator pos = _cli_options.find(option_name);
  if (pos != _cli_options.end())
  {
    for (unsigned int i = 0; i < pos->second.cli_switch.size(); ++i)
    {
      for (size_t j = 0; j < _argv.size(); j++)
      {
        auto arg = _argv[j];

        if (arg == pos->second.cli_switch[i])
        {
          // "Flag" CLI options added vector of Boolean types may apprear multiple times on the
          // command line (like a repeated verbosity flag to increase verbosity), when we see them
          // we append a true value to the vector.
          if (pos->second.argument_type == NONE)
            argument.push_back(T());
          else
            while (j + 1 < _argv.size() && _argv[j + 1][0] != '-' &&
                   _argv[j + 1].find("=") == std::string::npos)
            {
              std::stringstream ss;
              ss << _argv[j + 1];

              T item;
              setArgument(ss, item);
              argument.push_back(item);
              ++j;
            }
        }
      }
    }

    if (pos->second.required && argument.empty())
    {
      Moose::err << "Required parameter: " << option_name << " missing\n";
      printUsage();
    }
    return false;
  }
  mooseError("Unrecognized option name");
}
