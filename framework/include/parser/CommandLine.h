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

#include <vector>
#include <string>
#include <map>
#include <set>
#include "getpot.h"

// Forward Declaration
class Parser;

class CommandLine
{
public:
  struct Option
  {
    std::string description;
    std::vector<std::string> cli_syntax;
    bool required;
    bool optional_argument;
  };


  CommandLine(int argc, char * argv[]);
  virtual ~CommandLine();

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
  bool search(const std::string & option_name, std::string * argument = NULL);

  bool isVariableOnCommandLine(const std::string & name) const;

  /**
   * Print the usage info for this command line
   */
  void printUsage() const;

  /**
   * Get the GetPot object
   * @return Pointer to the GetPot object
   */
  GetPot * getPot() { return _get_pot; }

  /**
   * Check if we have a variable on the command line
   * @param name The name of the variable
   * @return True if the variable was defined on the command line
   */
  bool haveVariable(const std::string & name);

protected:
  /// Pointer to GetPot object that represents the command line arguments
  GetPot * _get_pot;
  /// Command line options
  std::map<std::string, Option> _cli_options;
  /// This is a set of all "extra" options on the command line
  std::set<std::string> _command_line_vars;
};



#endif //COMMANDLINE_H
