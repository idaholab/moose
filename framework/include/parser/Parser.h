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
#include "SyntaxFormatterInterface.h"

// libMesh
#include "getpot.h"
#include "exodusII_io.h"

class MooseMesh;
class MProblem;
class Executioner;

class Parser
{
public:
  enum SyntaxFormatterType
  {
    INPUT_FILE,
    YAML
  };

  Parser();

  virtual ~Parser();

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
   * This function initiates the traversal of the parse block tree which is each block is resposible
   * for creating and filling in various MOOSE based objects.
   */
  void execute();

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
   * Standard scripting languague trim function
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
   * Get the executioner
   */
  Executioner * getExecutioner();

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, InputParameters &p);

  /**
   * This function checks if all required params were filled/enter
   */
  void checkParams(const std::string & prefix, InputParameters &p);

  /**
   * prints a standard cli usage message
   */
  void printUsage() const;

  /**
   * Creates a syntax formatter for printing
   */
  void initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode, std::ostream & out = std::cout);

  /// Wrapper for syntax formatter print interface
  inline void print(const std::string & name, const std::string * prev_name, std::vector<InputParameters *> & param_ptrs)
  {
    mooseAssert(_syntax_formatter != NULL, "Syntax Formatter is NULL in Parser");
    _syntax_formatter->print(name, prev_name, param_ptrs);
  }

public:
  // data created while running execute()
  MooseMesh *_mesh;
  MooseMesh *_displaced_mesh;
  MProblem * _problem;
  Executioner *_executioner;

  ExodusII_IO *_exreader;                               ///< auxiliary object for restart
  bool _loose;                                          ///< true if parsing input file with loose syntax

  /// Functor for sorting input file syntax in MOOSE desired order
  class InputFileSort 
  {  
  public: 
    InputFileSort(); 
    bool operator() (Action *a, Action *b) const; 
    bool operator() (const std::pair<std::string, std::string> &a, const std::pair<std::string, std::string> &b) const;
    
  private:
    int sorter(const std::string &a, const std::string &b) const;
    std::vector<std::string> _o; 
  }; 


private:
  /**
   * This function initializes the command line options recognized by MOOSE based applications
   */
  void initOptions();

  /**
   * Searches the command line for the given option name (multiple syntaxes supported)
   */
  bool searchCommandLine(const std::string &option_name);
  
  /**
   * Use MOOSE Factories to construct a full parse tree for documentation or echoing input. 
   */
  void buildFullTree();
  
  /**
   * Helper functions for setting parameters of arbitrary types - bodies are in the .C file
   * since they are colled only from this Object
   */
  template<typename T>
  void setScalarParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<T>* param, bool in_global, GlobalParamsAction *global_block);
  
  
  template<typename T>
  void setVectorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<T> >* param, bool in_global, GlobalParamsAction *global_block);
  

  template<typename T>
  void setTensorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<std::vector<T> > >* param, bool in_global, GlobalParamsAction *global_block);

  /************************************
   * Private Data Members
   ************************************/
  struct CLIOption
  {
    std::string desc;
    std::vector<std::string> cli_syntax;
    bool required;
  };

  SyntaxFormatterInterface * _syntax_formatter;
  
  std::map<std::string, CLIOption> _cli_options;

  // Contains all of the sections that are not active during the parse phase so that blocks
  // nested more than one level deep can detect that the grandparent is not active
  std::set<std::string> _inactive_strings;

  bool _getpot_initialized;
  GetPot _getpot_file;
};

#endif //PARSER_H
