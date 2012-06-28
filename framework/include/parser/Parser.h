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

// libMesh
#include "getpot.h"
#include "exodusII_io.h"
#include "vector_value.h"
#include "tensor_value.h"

class ActionWarehouse;
class SyntaxTree;

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

  Parser(ActionWarehouse & action_wh);

  virtual ~Parser();

  /// Retrieve the Syntax associated with the passed Action and action_name
  std::string getSyntaxByAction(const std::string & action, const std::string & action_name) { return _syntax.getSyntaxByAction(action, action_name); }

  /**
   * Determines whether a particular block is marked as active
   * in the input file
   */
  bool isSectionActive(const std::string & section_name,
                       const std::map<std::string, std::vector<std::string> > & active_lists);

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
   * This function will split the passed in string on a set of delimiters appending the substrings
   * to the passed in vector.  The delimiters default to "/" but may be supplied as well.  In addition
   * if min_len is supplied, the minimum token length will be greater than the supplied value.
   */
  static void tokenize(const std::string &str,
                       std::vector<std::string> & elements,
                       unsigned int min_len = 1,
                       const std::string &delims = "/");

  /**
   * This function will escape all of the standard C++ escape character so they can be printed.  The
   * passed in parameter is modified in place
   */
  static void escape(std::string &str);

  /**
   * Standard scripting language trim function
   */
  static std::string trim(std::string str,
                          const std::string &white_space = " \t\n\v\f\r");

  /**
   * This function tokenizes a path and checks to see if it contains the string to look for
   */
  static bool pathContains(const std::string &expression,
                           const std::string &string_to_find,
                           const std::string &delims = "/");

  static void checkFileReadable(const std::string & filename, bool check_line_endings = false);

  static void checkFileWritable(const std::string & filename);

  /**
   * Return a reference to the getpot object to extract options from the input file
   */
  const GetPot * getPotHandle() const;

  /**
   * Set/Get a flag so that syntax dumped from the system is in alphabetical order
   */
  void setSortAlpha(bool sort_alpha_flag);
  bool getSortFlag() const;

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

protected:
  /// Action warehouse that will be filled by actions
  ActionWarehouse & _action_wh;
  /// Reference to an object that defines input file syntax
  Syntax & _syntax;

  /**
   * Helper functions for setting parameters of arbitrary types - bodies are in the .C file
   * since they are called only from this Object
   */

  template<typename T>
  void setScalarParameter(const std::string & full_name, const std::string & short_name,
                          InputParameters::Parameter<T>* param, bool in_global, GlobalParamsAction *global_block);

//  template<>
//  void setScalarParameter<MooseEnum>(const std::string & full_name, const std::string & short_name,
//                                     InputParameters::Parameter<MooseEnum>* param, bool in_global, GlobalParamsAction *global_block);


  void setRealVectorValue(const std::string & full_name, const std::string & short_name,
                          InputParameters::Parameter<RealVectorValue>* param, bool in_global, GlobalParamsAction *global_block);

  void setRealTensorValue(const std::string & full_name, const std::string & short_name,
                          InputParameters::Parameter<RealTensorValue>* param, bool in_global, GlobalParamsAction *global_block);

  template<typename T>
  void setVectorParameter(const std::string & full_name, const std::string & short_name,
                          InputParameters::Parameter<std::vector<T> >* param, bool in_global, GlobalParamsAction *global_block);

  SyntaxTree * _syntax_formatter;

  /// Contains all of the sections that are not active during the parse phase so that blocks
  /// nested more than one level deep can detect that the grandparent is not active
  std::set<std::string> _inactive_strings;

  bool _getpot_initialized;
  GetPot _getpot_file;
  std::string _input_filename;

  /// The set of all variables extracted from the input file
  std::set<std::string> _extracted_vars;
  bool _sort_alpha;

public:
  /**
   * Functor for sorting input file syntax in MOOSE desired order
   */
  class InputFileSort
  {
  public:
    InputFileSort(bool sort_alpha=false);
    bool operator() (Action *a, Action *b) const;
    bool operator() (const std::pair<std::string, Syntax::ActionInfo> &a, const std::pair<std::string, Syntax::ActionInfo> &b) const;

  private:
    int sorter(const std::string &a, const std::string &b) const;
    std::vector<std::string> _o;
    bool _sort_alpha;
  };
};



#endif //PARSER_H
