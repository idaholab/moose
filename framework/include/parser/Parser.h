#ifndef PARSER_H
#define PARSER_H

#include "ParserBlock.h"

class Parser
{
public:
  Parser(std::string input_filename);

  virtual ~Parser();
  
  /**
   * Parse an input file consisting of getpot syntax and setup objects
   * in the MOOSE derived application
   */
  void parse();

private:
  /**
   * This function will split the passed in string on a set of delimiters appending the substrings
   * to the passed in vector.  The delimiters default to "/" but may be supplied as well
   */
  static void tokenize(const std::string &str,
                       std::vector<std::string> & elements,
                       const std::string &delims = "/");

  /**
   * This function initiates the traversal of the parse block tree which is each block is resposible
   * for creating and filling in various MOOSE based objects.
   */
  void execute();

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, Parameters &p, const GetPot & input_file);

  /**
   * This function is the helper function for extractParams and does the actual extraction
   * from the input file
   */
  bool setParameters(std::string name, Parameters::iterator &it, const GetPot &input_file);
  
  std::string _input_filename;
  std::vector<std::string> _section_names;

  /**
   * Pointer to the parser block tree built from the call to "parse"
   */
  ParserBlock *_input_tree;
};

#endif //PARSER_H
