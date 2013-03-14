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

#include "Moose.h"
#include "Parser.h"
#include "MooseSyntax.h"
#include "ActionWarehouse.h"
#include "SystemInfo.h"
#include "Factory.h"
#include "ActionFactory.h"

class Executioner;
class MooseApp;

template<>
InputParameters validParams<MooseApp>();

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
  virtual ~MooseApp();

  /**
   * Get the name of the object
   * @return The name of the object
   */
  const std::string & name() { return _name; }

  /**
   * Get the parameters of the object
   * @return The parameters of the object
   */
  InputParameters & parameters() { return _pars; }

  /**
   * Retrieve a parameter for the object
   * @param name The name of the parameter
   * @return The value of the parameter
   */
  template <typename T>
  const T & getParam(const std::string & name) { return _pars.get<T>(name); }

  /**
   * Retrieve a parameter for the object (const version)
   * @param name The name of the parameter
   * @return The value of the parameter
   */
  template <typename T>
  const T & getParam(const std::string & name) const { return _pars.get<T>(name); }

  inline bool isParamValid(const std::string &name) const { return _pars.isParamValid(name); }

  /**
   * Run the application
   */
  virtual void run();

  /**
   * Setup options based on InputParameters.
   */
  virtual void setupOptions();

  ActionWarehouse & actionWarehouse() { return _action_warehouse; }

  Parser & parser() { return _parser; }

  Syntax & syntax() { return _syntax; }

  /**
   * Set the input file name.
   */
  void setInputFileName(std::string input_file_name);

  /**
   * Override the selection of the output file base name.
   */
  void setOutputFileBase(std::string output_file_base) { _output_file_base = output_file_base; }

  /**
   * Override the selection of the output file base name.
   */
  std::string getOutputFileBase() { return _output_file_base; }

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

  std::string getSysInfo();

  /**
   * Retrieve the Executioner for this App.
   */
  Executioner * getExecutioner() { return _executioner; }

  /**
   * Retrieve the Factory associated with this App.
   */
  Factory & getFactory() { return _factory; }

  /**
   * Retrieve the ActionFactory associated with this App.
   */
  ActionFactory & getActionFactory() { return _action_factory; }

  /**
   * Get the command line
   * @return The reference to the command line object
   * Setup options based on InputParameters.
   */
  CommandLine * commandLine() { return _command_line; }


  /**
   * Actually build everything in the input file.
   */
  virtual void runInputFile();

  /**
   * Execute the Executioner that was built.
   */
  virtual void executeExecutioner();

protected:
  MooseApp(const std::string & name, InputParameters parameters);

  virtual void meshOnly(std::string mesh_file_name);

  /// The name of this object
  std::string _name;

  /// Parameters of this object
  InputParameters _pars;

  /// Input file name used
  std::string _input_filename;

  /// The output file basename
  std::string _output_file_base;

  /// Command line object
  CommandLine * _command_line;

  /// Syntax of the input file
  Syntax _syntax;
  /// The Factory responsible for building Actions
  ActionFactory _action_factory;
  /// Where built actions are stored
  ActionWarehouse _action_warehouse;
  /// Parser for parsing the input file
  Parser _parser;
  /// Pointer to the executioner of this run (typically build by actions)
  Executioner * _executioner;
  /// System Information
  SystemInfo * _sys_info;

  /// Indicates whether warnings, errors, or no output is displayed when unused parameters are detected
  enum UNUSED_CHECK { OFF, WARN_UNUSED, ERROR_UNUSED } _enable_unused_check;

  Factory _factory;

protected:
  /// Indicates whether warnings or errors are displayed when overridden parameters are detected
  bool _error_overridden;
  bool _ready_to_exit;
};

#endif /* MOOSEAPP_H */
