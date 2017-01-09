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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

// Moose Includes
#include "MooseError.h"

// libMesh includes
#include "libmesh/getpot.h"

// C++ includes
#include <vector>
#include <string>
#include <map>
#include <set>

// Forward Declaration
class InputParameters;

/**
 * This class wraps a GetPot object associated with the command line
 * used to run the code.
 */
class CommandLine
{
public:
  /// Type of argument for a given option
  enum ARGUMENT {
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
   * This function extracts parameters from the command line in name=value format.  Note
   * that the name should be fully qualified (i.e. BCs/left/value=10)
   */
  void buildVarsSet();

  /**
   * This routine searches the command line for the given option "handle"
   * and returns a boolean indicating whether it was found.  If the given
   * option has an argument it is also filled in.
   */
  bool search(const std::string & option_name);

  template <typename T>
  bool search(const std::string & option_name, T & argument);

  bool isVariableOnCommandLine(const std::string & name) const;

  /**
   * Print the usage info for this command line
   */
  void printUsage() const;

  /**
   * Get the GetPot object
   * @return Pointer to the GetPot object
   */
  GetPot * getPot() { return _get_pot.get(); }

  /**
   * Check if we have a variable on the command line. Note that a call to this
   * method can modify the prefix unless the optional Boolean is set to false.
   *
   * @param name The name of the variable
   * @return True if the variable was defined on the command line
   */
  bool haveVariable(const std::string & name, bool allow_prefix_change=true);

  /**
   * Sets the prefix for the CommandLine object. This is used for passing
   * parameters to Multiapps
   */
  void setPrefix(const std::string & name, const std::string & num="");

  /**
   * Resets the prefix to the value set with the last call to setPrefix.
   * Generally you do not need to call this method unless you wish
   * to reset the prefix after calling haveVariable before retrieving a
   * raw pointer to the GetPot object.
   */
  void resetPrefix();

  // Dump the contents of the GetPot object
  void print(const char * prefix, std::ostream & out_stream, unsigned int skip_count);

protected:
  /// Pointer to GetPot object that represents the command line arguments
  std::unique_ptr<GetPot> _get_pot;
  /// Command line options
  std::map<std::string, Option> _cli_options;
  /// This is a set of all "extra" options on the command line
  std::set<std::string> _command_line_vars;

  /// The base prefix for this CommandLine object
  std::string _base_prefix;
  /// The number added to the prefix to point it at a specific Multiapp
  std::string _prefix_num;
  /// Boolean indicating whether we have prefixes set on this CommandLine Object
  bool _has_prefix;
};

template <typename T>
bool CommandLine::search(const std::string &option_name, T & argument)
{
  std::map<std::string, Option>::iterator pos = _cli_options.find(option_name);
  if (pos != _cli_options.end())
  {
    for (unsigned int i=0; i<pos->second.cli_switch.size(); ++i)
    {
      if (_get_pot->search(pos->second.cli_switch[i]))
      {
        // "Flag" CLI options are added as Boolean types, when we see them
        // we set the Boolean argument to true
        if (pos->second.argument_type == NONE)
          argument = true;
        else
          argument = _get_pot->next(argument);
        return true;
      }
    }

    if (pos->second.required)
    {
      Moose::err << "Required parameter: " << option_name << " missing\n";
      printUsage();
    }
  }
  else
    mooseError("Unrecognized option name");

  return false;
}



#endif //COMMANDLINE_H
