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

#ifndef PARSER_H
#define PARSER_H

#include "GlobalParamsAction.h"
#include "MooseSyntax.h"
#include "CommandLine.h"
#include "MooseEnum.h"
#include "Syntax.h"

// libMesh
#include "libmesh/getpot.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

class ActionWarehouse;
class SyntaxTree;
class MooseApp;
class Factory;
class ActionFactory;

#define DEBUG_PARSER 1

/**
 * Class for parsing input files.
 */
class Parser
{
public:
  enum SyntaxFormatterType
  {
    INPUT_FILE,
    YAML
  };

  Parser(MooseApp & app, ActionWarehouse & action_wh);

  virtual ~Parser();

  /// Retrieve the Syntax associated with the passed Action and task
  std::string getSyntaxByAction(const std::string & action, const std::string & task) { return _syntax.getSyntaxByAction(action, task); }

  /**
   * Determines whether a particular block is marked as active
   * in the input file
   */
  bool isSectionActive(const std::string & section_name,
                       const std::map<std::string, std::vector<std::string> > & active_lists);

  /**
   * Return the filename that was parsed
   */
  std::string getFileName(bool stripLeadingPath = true) const;

  /**
   * Parse an input file consisting of getpot syntax and setup objects
   * in the MOOSE derived application
   */
  void parse(const std::string &input_filename);

  /**
   * This function checks to make sure that the active lists (active=*) are used up in the supplied
   * input file.
   */
  void checkActiveUsed(std::vector<std::string > & sections,
                       const std::map<std::string, std::vector<std::string> > & active_lists);

  /**
   * Return a reference to the getpot object to extract options from the input file
   */
  const GetPot * getPotHandle() const;

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, InputParameters &p);

  /**
   * Creates a syntax formatter for printing
   */
  void initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode);

  /**
   * Use MOOSE Factories to construct a full parse tree for documentation or echoing input.
   */
  void buildFullTree(const std::string &search_string);

  /**
   * This function checks to see if there are unidentified variables in the input file (i.e. unused)
   * If the warn_is_error is set, then the program will abort if unidentified parameters are found
   */
  void checkUnidentifiedParams(std::vector<std::string> & all_vars, bool error_on_warn);

  /**
   * This function checks to see if there were any overridden parameters in the input file.
   * (i.e. suplied more than once)
   * @param error_on_warn a Boolean that will trigger an error if this case is detected
   */
  void checkOverriddenParams(bool error_on_warn);

protected:
  /// The MooseApp this Parser is part of
  MooseApp & _app;
  /// The Factory associated with that MooseApp
  Factory & _factory;
  /// Action warehouse that will be filled by actions
  ActionWarehouse & _action_wh;
  /// The Factory that builds actions
  ActionFactory & _action_factory;
  /// Reference to an object that defines input file syntax
  Syntax & _syntax;

  /**
   * Helper functions for setting parameters of arbitrary types - bodies are in the .C file
   * since they are called only from this Object
   */
  template<typename T>
  void setScalarParameter(const std::string & full_name, const std::string & short_name,
                          InputParameters::Parameter<T>* param, bool in_global, GlobalParamsAction *global_block);

  template<typename T>
  void setVectorParameter(const std::string & full_name, const std::string & short_name,
                          InputParameters::Parameter<std::vector<T> >* param, bool in_global, GlobalParamsAction *global_block);

  template<typename T>
  void setScalarComponentParameter(const std::string & full_name, const std::string & short_name,
                                   InputParameters::Parameter<T> * param, bool in_global, GlobalParamsAction * global_block);

  template<typename T>
  void setVectorComponentParameter(const std::string & full_name, const std::string & short_name,
                                   InputParameters::Parameter<std::vector<T> > * param, bool in_global, GlobalParamsAction * global_block);


  SyntaxTree * _syntax_formatter;

  /// Contains all of the sections that are not active during the parse phase so that blocks
  /// nested more than one level deep can detect that the grandparent is not active
  std::set<std::string> _inactive_strings;

  bool _getpot_initialized;
  GetPot _getpot_file;
  std::string _input_filename;

  /// The set of all variables extracted from the input file
  std::set<std::string> _extracted_vars;

  bool _sections_read;
};



#endif //PARSER_H
