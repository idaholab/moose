#ifndef PARSERBLOCK_H
#define PARSERBLOCK_H

#include <vector>

#include "MooseSystem.h"
#include "ValidParams.h"
#include "Parser.h"

#include "getpot.h"

//Forward Declarations
class Parser;
class ParserBlock;

template<>
InputParameters validParams<ParserBlock>();

/**
 * This class represents the base class for the various parser block data structures and the associated
 * member function which is responsible for setting up the appropriate data structures within MOOSE
 */
class ParserBlock 
{
public:
  /**
   * Typedef to hide implementation details
   */
  typedef std::vector<ParserBlock *>::iterator PBChildIterator;
  
  /**
   * The registered id (reg_id) is a "path-like" identifier supporting an optional trailing
   * wildcard character '*'.  The real_id is the actual parsed path containing the real identifier
   * for this ParserBlock instance.  A pointer to the parent is passed for use in searching the tree
   * Finally a reference to the GetPot object is passed for flexible extension.
   */
  ParserBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  /**
   * Cleans up the ParserBlock tree strucutre
   */
  virtual ~ParserBlock();

  /**
   * Returns the full path identifier for this block
   */
  inline std::string getID() const { return _name; }

  /**
   * Returns the short name which is the final string after the last delimiter for the
   * current ParserBlock
   */
  std::string getShortName() const;

  /**
   * This is a helper function which extracts the the paramater "type" from the inputfile
   * useful for passing to Factory classes
   */
  std::string getType() const;

  /**
   * Check to see if the passed in string is active
   */
  bool checkActive(const std::string &name) const;
  
  /**
   * This is the workhorse function which must setup the appropriate MOOSE objects corresponding
   * to this ParserBlock.  Children blocks must be visited explicitly but may be done with a call
   * to "visitChildren"
   */
  virtual void execute();

  void driveExecute();

  inline void addPrereq(std::string name)
    {
      _execute_prereqs.insert(name);
    }

  /************************************
   * Data Accessors
   ************************************/
  template <typename T>
  T getParamValue(std::string name) const
    {
      return _block_params.get<T>(name);
    }

  inline bool isParamValid(const std::string &name) const
    {
      return _block_params.isParamValid(name);
    }
  
  inline void setClassParams(InputParameters p)
    {
      _class_params = p;
    }

  inline InputParameters & getClassParams()
    {
      return _class_params;
    }

  inline InputParameters & getBlockParams()
    {
      return _block_params;
    }
  

  /************************************
   * Public Data Members
   ************************************/
  /**
   * Child Parser Blocks and parent pointers (TODO: Probably should be hidden)
   */
  std::vector<ParserBlock *> _children;
  ParserBlock * _parent;

  /**
   * This function searches the ParserTree for a given ParserBlock and returns the handle to it
   * if it exists.  This function expects a long name not a short name.
   */
  ParserBlock * locateBlock(const std::string & id);

  void printBlockData();

  /**
   * This function returns the number of active children which is either the children named in the
   * optional "active" parameter or else all of the children underneath this Block
   */
  unsigned int n_activeChildren() const;

protected:
  /**
   * The MooseSystem this parser is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * This function calles execute over all of the child blocks of the current Parser Block
   */
  void visitChildren(void (ParserBlock::*action)() = &ParserBlock::execute,
                     bool visit_active_only=true,
                     bool check_prereqs=true);

  /**
   * This function checks the prereqs of the the passed ParserBlock * to make sure that it
   * can be executed.
   */
  bool checkPrereqs(ParserBlock *pb_ptr);

  void executeDeferred(void (ParserBlock::*action)());
  
  /************************************
   * Protected Data Members
   ************************************/
//  std::string _reg_id;
//  std::string _real_id;
  std::string _name;
  std::string _block_name;
  Parser & _parser_handle;
  const GetPot * _getpot_handle;
  std::vector<std::string> _active;

  /**
   * The list of ParserBlocks which must be executed prior to executing the current ParserBlock
   */
  std::set<std::string> _execute_prereqs;
  

  /************************************
   * Private Data Members (use accessors)
   ************************************/
private:
  /**
   * The _block_params are those parameters which are valid for the currently parsed block but may not
   * necessarily be passed directly to the Factory constructor for this object type.  These should
   * be set in the constructor for each derived class and are an augmentation to the _class_params.
   */
  InputParameters _block_params;

  /** The _class_params are those parameters which will be passed directly to the Factory constructor
   * objects directly.
   */
  InputParameters _class_params;
  
//  std::map<std::string, std::string> _doc_string;
//  std::set<std::string> _required_params;
};

#endif //PARSERBLOCK_H
