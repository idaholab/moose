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

#ifndef MOOSEAPP_H
#define MOOSEAPP_H

#include <iostream>
#include <vector>
#include <map>
#include <set>

#include "Parser.h"
#include "MooseSyntax.h"
#include "ActionWarehouse.h"
#include "Executioner.h"
#include "SystemInfo.h"

/**
 * Base class for MOOSE-based applications
 *
 * This generic class for application provides:
 * - parsing command line arguments,
 * - parsing an input file,
 * - executing the simulation
 *
 * Each application should register its own objects and register its own special syntax
 */
class MooseApp
{
public:
  MooseApp(int argc, char *argv[]);
  virtual ~MooseApp();

  /**
   * Register objects and associate syntax
   */
  virtual void init();

  /**
   * Run the application
   */
  virtual void run();

  /**
   * Build the command line options
   */
  virtual void initCommandLineOptions();

  /**
   * Parse the command line options
   */
  virtual void parseCommandLine();

  /**
   * Get the command line
   * @return The reference to the command line object
   */
  CommandLine & commandLine() { return _command_line; }

  ActionWarehouse & actionWarehouse() { return _action_warehouse; }

  Parser & parser() { return _parser; }

  Syntax & syntax() { return _syntax; }

  /**
   * Return the filename that was parsed
   */
  std::string getFileName(bool stripLeadingPath = true) const;

  /**
   * Set a flag so that the parser will either warn or error when unused variables are seen after
   * parsing is complete.
   */
  void setCheckUnusedFlag(bool warn_is_error = false);

  /**
   * Set a flag so that the parser will throw an error if overriden parameters are detected
   */
  void setErrorOverridden();

  /**
   * Set/Get a flag so that syntax dumped from the system is in alphabetical order
   */
  void setSortAlpha(bool sort_alpha_flag) { _parser.setSortAlpha(sort_alpha_flag); }
  bool getSortFlag() const { return _parser.getSortFlag(); }

  /**
   * Removes warnings and error checks for unrecognized variables in the input file
   */
  void disableCheckUnusedFlag();

  std::string getSysInfo() { return _sys_info.getInfo(); }

protected:
  virtual void runInputFile();

  /// Input file name used
  std::string _input_filename;
  /// Command line object
  CommandLine _command_line;
  /// Syntax of the input file
  Syntax _syntax;
  ///
  ActionWarehouse _action_warehouse;
  /// Parser for parsing the input file
  Parser _parser;
  /// Pointer to the executioner of this run (typically build by actions)
  Executioner * _executioner;
  /// System Information
  SystemInfo _sys_info;

  /// Indicates whether warnings, errors, or no output is displayed when unused parameters are detected
  enum UNUSED_CHECK { OFF, WARN_UNUSED, ERROR_UNUSED } _enable_unused_check;

private:
  /// Inidicates whether warnings or errors are displayed when overrridden parameters are detected
  bool _error_overridden;
};


namespace Moose
{
  extern MooseApp * app;
}

#endif /* MOOSEAPP_H */
