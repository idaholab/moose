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

#ifndef PARSER_H_
#define PARSER_H_

// Set to 1 to use Actions, 0 for ParserBlocks
#define PARSER_ACTION 1 

#include <list>

#include "GlobalParamsAction.h"

// libMesh
#include "getpot.h"
#include "exodusII_io.h"

class MooseMesh;
class MProblem;
class Executioner;


class Parser
{
public:
  /**
   * Default constructor that initializes the parser and looks for the option to dump
   * the registered objects
   */
  Parser(const std::string &dump_string="--dump");

  /**
   * Destructor to remove the dynamically generated Parse Tree
   */
  virtual ~Parser();

  /**
   * Determines whether a particular block is marked as active
   * in the input file
   */
  bool isSectionActive(const std::string & section_name,
                       const std::map<std::string, std::vector<std::string> > & active_lists) const;
  
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
   * to the passed in vector.  The delimiters default to "/" but may be supplied as well
   */
  static void tokenize(const std::string &str,
                       std::vector<std::string> & elements,
                       const std::string &delims = "/");

  /**
   * This function tokenizes a path and checks to see if it contains the string to look for
   */
  static bool pathContains(const std::string &expression,
                           const std::string &string_to_find,
                           const std::string &delims = "/");
    
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
   * prints a standard cli usage message
   */
  void printUsage() const;

  static void checkFileReadable(const std::string & filename);
  
  static void checkFileWritable(const std::string & filename);

public:
  // data created while running execute()
  MooseMesh *_mesh;
  MProblem * _problem;
  Executioner *_executioner;

  ExodusII_IO *_exreader;                               /// auxiliary object for restart
  bool _loose;                                          /// true if parsing input file with loose syntax

private:
  /**
   * This function inserts blocks into the tree which are optional in the input file but are
   * necessary for the correct execution of MOOSE based applications.
   */
  void fixupOptionalBlocks();

  /**
   * Use MOOSE Factories to construct a full parse tree for documentation. Format
   * parameter specifies how to print the resulting parse tree. Only "dump", the
   * original human readable format, and "yaml" are supported.
   */
  void buildFullTree( const std::string format );

  void printInputParameterData(const std::string & name, const std::string * type, std::vector<InputParameters *> & param_ptrs);
  
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
  std::string _input_filename;
  std::vector<std::string> _section_names;
  const std::string _dump_string;
  const static std::string _show_tree;

  bool _getpot_initialized;
  bool _tree_printed;
  GetPot _getpot_file;
};

#endif //PARSER_H
