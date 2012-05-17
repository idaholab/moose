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

// Forward Declaration
class Parser;

class CommandLine
{
public:
  CommandLine(Parser &parser);

  /**
   * This routine parses the command line looking for and handling various options.
   *
   * @return The inputfile file name is returned.
   */
  std::string parseCommandLine();

  /**
   * This function extracts parameters from the command line in name=value format.  Note
   * that the name should be fully qualified (i.e. BCs/left/value=10)
   */
  void buildCommandLineVarsSet();

  /**
   * This routine searches the command line for the given option "handle"
   * and returns a boolean indicating whether it was found.  If the given
   * option has an argument it is also filled in.
   */
  bool searchCommandLine(const std::string &option_name, std::string *argument=NULL);

  bool isVariableOnCommandLine(const std::string &name) const;

  void printUsage() const;

protected:
  struct CLIOption
  {
    std::string description;
    std::vector<std::string> cli_syntax;
    bool required;
    bool optional_argument;
  };

  Parser &_parser;
  std::map<std::string, CLIOption> _cli_options;

  /// This is a set of all "extra" options on the command line
  std::set<std::string> _command_line_vars;
};



#endif //COMMANDLINE_H
