//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ConsoleStreamInterface.h"
#include "MooseTypes.h"
#include "InputParameters.h"
#include "Syntax.h"

#include "hit.h"

#include <vector>
#include <string>
#include <iomanip>

// Forward declarations
class ActionWarehouse;
class SyntaxTree;
class MooseApp;
class Factory;
class ActionFactory;
class GlobalParamsAction;
class JsonSyntaxTree;

class FuncParseEvaler : public hit::Evaler
{
public:
  virtual std::string
  eval(hit::Field * n, const std::list<std::string> & args, hit::BraceExpander & exp);
};

class UnitsConversionEvaler : public hit::Evaler
{
public:
  virtual std::string
  eval(hit::Field * n, const std::list<std::string> & args, hit::BraceExpander & exp);
};

/**
 * Class for parsing input files. This class utilizes the GetPot library for actually tokenizing and
 * parsing files. It is not currently designed for extensibility. If you wish to build your own
 * parser, please contact the MOOSE team for guidance.
 */
class Parser : public ConsoleStreamInterface, public hit::Walker
{
public:
  enum SyntaxFormatterType
  {
    INPUT_FILE,
    YAML
  };

  Parser(MooseApp & app, ActionWarehouse & action_wh);

  virtual ~Parser();

  /**
   * Return the primary (first) filename that was parsed
   */
  std::string getPrimaryFileName(bool stripLeadingPath = true) const;

  /**
   * Parse an input file (or text string if non-empty) consisting of hit syntax and setup objects
   * in the MOOSE derived application
   */
  void parse(const std::vector<std::string> & input_filenames, const std::string & input_text = "");

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

  void walk(const std::string & fullpath, const std::string & nodepath, hit::Node * n);

  void errorCheck(const Parallel::Communicator & comm, bool warn_unused, bool err_unused);

  std::vector<std::string> listValidParams(std::string & section_name);

  /**
   * Marks MOOSE hit syntax from supplied command-line arguments
   */
  std::string hitCLIFilter(std::string appname, const std::vector<std::string> & argv);

protected:
  /**
   * Helper functions for setting parameters of arbitrary types - bodies are in the .C file
   * since they are called only from this Object
   */
  /// Template method for setting any scalar type parameter read from the input file or command line
  template <typename T, typename Base>
  void setScalarParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<T> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  template <typename T, typename UP_T, typename Base>
  void setScalarValueTypeParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /// Template method for setting any vector type parameter read from the input file or command line
  template <typename T, typename Base>
  void setVectorParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<std::vector<T>> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  /// Template method for setting any map type parameter read from the input file or command line
  template <typename KeyType, typename MappedType>
  void setMapParameter(const std::string & full_name,
                       const std::string & short_name,
                       InputParameters::Parameter<std::map<KeyType, MappedType>> * param,
                       bool in_global,
                       GlobalParamsAction * global_block);

  /**
   * Sets an input parameter representing a file path using input file data.  The file path is
   * modified to be relative to the directory this application's input file is in.
   */
  template <typename T>
  void setFilePathParam(const std::string & full_name,
                        const std::string & short_name,
                        InputParameters::Parameter<T> * param,
                        InputParameters & params,
                        bool in_global,
                        GlobalParamsAction * global_block);

  /**
   * Sets an input parameter representing a vector of file paths using input file data.  The file
   * paths are modified to be relative to the directory this application's input file is in.
   */
  template <typename T>
  void setVectorFilePathParam(const std::string & full_name,
                              const std::string & short_name,
                              InputParameters::Parameter<std::vector<T>> * param,
                              InputParameters & params,
                              bool in_global,
                              GlobalParamsAction * global_block);
  /**
   * Template method for setting any double indexed type parameter read from the input file or
   * command line.
   */
  template <typename T>
  void setDoubleIndexParameter(const std::string & full_name,
                               const std::string & short_name,
                               InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                               bool in_global,
                               GlobalParamsAction * global_block);

  /**
   * Template method for setting any triple indexed type parameter read from the input file or
   * command line.
   */
  template <typename T>
  void setTripleIndexParameter(
      const std::string & full_name,
      const std::string & short_name,
      InputParameters::Parameter<std::vector<std::vector<std::vector<T>>>> * param,
      bool in_global,
      GlobalParamsAction * global_block);

  /**
   * Template method for setting any multivalue "scalar" type parameter read from the input file or
   * command line.  Examples include "Point" and "RealVectorValue".
   */
  template <typename T>
  void setScalarComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /**
   * Template method for setting several multivalue "scalar" type parameter read from the input
   * file or command line.  Examples include "Point" and "RealVectorValue".
   */
  template <typename T>
  void setVectorComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<std::vector<T>> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /**
   * Template method for setting vector of several multivalue "scalar" type parameter read from the
   * input file or command line.  Examples include vectors of several "Point"s and
   * "RealVectorValue"s such as (a three-element vector; each element is several "Point"s):
   * points_values = '0 0 0
   *                  0 0 1;
   *                  0 1 0;
   *                  1 0 0
   *                  1 1 0
   *                  1 1 1'
   */
  template <typename T>
  void
  setVectorVectorComponentParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block);

  std::unique_ptr<hit::Node> _cli_root = nullptr;
  std::unique_ptr<hit::Node> _root = nullptr;
  std::vector<std::string> _secs_need_first;

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
  std::unique_ptr<SyntaxTree> _syntax_formatter;

  /// The input file names that are used for parameter extraction
  std::vector<std::string> _input_filenames;

  /// The set of all variables extracted from the input file
  std::set<std::string> _extracted_vars;

  /// Boolean to indicate whether parsing has started (sections have been extracted)
  bool _sections_read;

  /// The current parameter object for which parameters are being extracted
  InputParameters * _current_params;

  /// The current stream object used for capturing errors during extraction
  std::ostringstream * _current_error_stream;

private:
  std::string _errmsg;
  std::string _warnmsg;
  void walkRaw(std::string fullpath, std::string nodepath, hit::Node * n);

  // Allow the MooseServer class to access the root node of the hit parse tree
  friend class MooseServer;
};
