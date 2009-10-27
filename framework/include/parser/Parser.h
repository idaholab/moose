#ifndef PARSER_H
#define PARSER_H

#include "ParserBlock.h"

class Parser
{
public:
  /**
   * This is the normal user constructor that takes a filename in Getpot format to parse.
   * This constructor will build the input parser tree automatically
   */
  Parser(std::string input_filename);

  /**
   * This constructor is for building a tree of all possible elements for use in building
   * documentation and help
   */
  Parser(bool build_full_tree);

  /**
   * Destructor to remove the dynamically generated Parse Tree
   */
  virtual ~Parser();

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
   * Return a reference to the getpot object to extract options from the input file
   */
  const GetPot * getPotHandle() const;
  
private:
  /**
   * Default Constructor not allowed
   */
  Parser();
  
  /**
   * This function inserts blocks into the tree which are optional in the input file but are
   * necessary for the correct execution of MOOSE based applications.
   */
  void fixupOptionalBlocks();

  /**
   * Parse an input file consisting of getpot syntax and setup objects
   * in the MOOSE derived application
   */
  void parse();

  /**
   * Use MOOSE Factories to construct a full parse tree for documentation
   */
  void buildFullTree();

  /**
   * This function check for the existance and readability of the input file and throws a
   * MOOSE error if there is a problem
   */
  void checkInputFile();
  
  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, Parameters &p);

  /**
   * This function is the helper function for extractParams and does the actual extraction
   * from the input file
   */
  bool setParameters(std::string name, Parameters::iterator &it);


  /************************************
   * Private Data Members
   ************************************/
  std::string _input_filename;
  std::vector<std::string> _section_names;

  /**
   * Pointer to the parser block tree built from the call to "parse"
   */
  ParserBlock *_input_tree;
  
  bool _getpot_initialized;
  GetPot _getpot_file;
};

#endif //PARSER_H
