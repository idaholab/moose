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

// MOOSE includes
#include "ConsoleStreamInterface.h"
#include "MooseTypes.h"
#include "InputParameters.h"
#include "Syntax.h"

// libMesh include
#include "libmesh/getpot.h"

// Forward declarations
class ActionWarehouse;
class SyntaxTree;
class MooseApp;
class Factory;
class ActionFactory;
class GlobalParamsAction;
class JsonSyntaxTree;

/**
 * Class for parsing input files. This class utilizes the GetPot library for actually tokenizing and
 * parsing files. It is not
 * currently designed for extensibility. If you wish to build your own parser, please contact the
 * MOOSE team for guidance.
 */
class Parser : public ConsoleStreamInterface
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
  std::string getSyntaxByAction(const std::string & action, const std::string & task)
  {
    return _syntax.getSyntaxByAction(action, task);
  }

  /**
   * Determines whether a particular block is marked as active
   * in the input file
   */
  bool isSectionActive(const std::string & section_name,
                       const std::map<std::string, std::vector<std::string>> & active_lists);

  /**
   * Return the filename that was parsed
   */
  std::string getFileName(bool stripLeadingPath = true) const;

  /**
   * Parse an input file consisting of getpot syntax and setup objects
   * in the MOOSE derived application
   */
  void parse(const std::string & input_filename);

  /**
   * This function checks to make sure that the active lists (active=*) are used up in the supplied
   * input file.
   */
  void checkActiveUsed(std::vector<std::string> & sections,
                       const std::map<std::string, std::vector<std::string>> & active_lists);

  /**
   * Return a reference to the getpot object to extract options from the input file
   */
  const GetPot * getPotHandle() const;

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, InputParameters & p);

  /**
   * Creates a syntax formatter for printing
   */
  void initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode);

  /**
   * Use MOOSE Factories to construct a full parse tree for documentation or echoing input.
   */
  void buildFullTree(const std::string & search_string);

  /**
   * Use MOOSE Factories to construct a parameter tree for documentation or echoing input.
   */
  void buildJsonSyntaxTree(JsonSyntaxTree & tree) const;

  /**
   * This function checks to see if there are unidentified variables in the input file (i.e. unused)
   * If the warn_is_error is set, then the program will abort if unidentified parameters are found
   */
  void checkUnidentifiedParams(std::vector<std::string> & all_vars,
                               bool error_on_warn,
                               bool in_input_file,
                               std::shared_ptr<FEProblemBase> fe_problem) const;

  /**
   * This function checks to see if there were any overridden parameters in the input file.
   * (i.e. supplied more than once)
   * @param error_on_warn a Boolean that will trigger an error if this case is detected
   */
  void checkOverriddenParams(bool error_on_warn) const;

protected:
  /// Appends sections from the CLI Reorders section names so that Debugging options can be enabled before parsing begins
  void appendAndReorderSectionNames(std::vector<std::string> & section_names);

  /// Reorders specified tasks in the section names list (helper method called from appednAndReorderSectionNames
  void reorderHelper(std::vector<std::string> & section_names,
                     const std::string & action,
                     const std::string & task) const;

  /**
   * Helper functions for setting parameters of arbitrary types - bodies are in the .C file
   * since they are called only from this Object
   */
  /// Template method for setting any scalar type parameter read from the input file or command line
  template <typename T>
  void setScalarParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<T> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  template <typename T, typename UP_T>
  void setScalarValueTypeParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /// Template method for setting any vector type parameter read from the input file or command line
  template <typename T>
  void setVectorParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<std::vector<T>> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  /// Template method for setting any double indexed type parameter read from the input file or command line
  template <typename T>
  void setDoubleIndexParameter(const std::string & full_name,
                               const std::string & short_name,
                               InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                               bool in_global,
                               GlobalParamsAction * global_block);

  /// Template method for setting any multivalue "scalar" type parameter read from the input file or command line.  Examples include "Point" and "RealVectorValue"
  template <typename T>
  void setScalarComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /// Template method for setting several multivalue "scalar" type parameter read from the input file or command line.  Examples include "Point" and "RealVectorValue"
  template <typename T>
  void setVectorComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<std::vector<T>> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

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

  /// Object for holding the syntax parse tree
  SyntaxTree * _syntax_formatter;

  /// Contains all of the sections that are not active during the parse phase so that blocks
  /// nested more than one level deep can detect that the grandparent is not active
  std::set<std::string> _inactive_strings;

  /// Boolean indicating whether the getpot parser has been initialized
  bool _getpot_initialized;

  /// The getpot object used for extracting parameters
  GetPot _getpot_file;

  /// The getpot object used for testing
  GetPot _getpot_file_error_checking;

  /// The input file name that is used for parameter extraction
  std::string _input_filename;

  /// The set of all variables extracted from the input file
  std::set<std::string> _extracted_vars;

  /// Boolean to indicate whether parsing has started (sections have been extracted)
  bool _sections_read;

  /// The current parameter object for which parameters are being extracted
  InputParameters * _current_params;

  /// The current stream object used for capturing errors during extraction
  std::ostringstream * _current_error_stream;
};

#endif // PARSER_H
