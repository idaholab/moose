#ifndef PARSERBLOCK_H
#define PARSERBLOCK_H

#include "Kernel.h"

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
  ParserBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  /**
   * Cleans up the ParserBlock tree strucutre
   */
  virtual ~ParserBlock();

  /**
   * Returns the full path identifier for this block
   */
  inline std::string getID() const { return _real_id; }

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
   * This is the workhorse function which must setup the appropriate MOOSE objects corresponding
   * to this ParserBlock.  Children blocks must be visited explicitly but may be done with a call
   * to "visitChildren"
   */
  virtual void execute();

  /**
   * This function is for adding new parameters to the ParserBlock that will be extracted from
   * the input file, checked and used within MOOSE
   */
  template <typename T>
  void addParam(std::string name, T value, std::string doc_string, bool required) 
    {
      _block_params.set<T>(name) = value;
      if (required)
        _required_params.insert(name);
      _doc_string[name] = doc_string;
    }

  /**
   * This function is for adding new parameters to the ParserBlock that will be extracted from
   * the input file, it's intended purpose is for required parameters with no default value
   */
  template <typename T>
  void addParam(std::string name, std::string doc_string, bool required) 
    {
      _block_params.set<T>(name);
      if (required)
        _required_params.insert(name);
      _doc_string[name] = doc_string;
    }

  /************************************
   * Data Accessors
   ************************************/
  template <typename T>
  T getParamValue(std::string name) const
    {
      return _block_params.get<T>(name);
    }
  
  inline void setClassParams(Parameters p)
    {
      _class_params = p;
    }

  inline Parameters & getClassParams()
    {
      return _class_params;
    }

  inline Parameters & getBlockParams()
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
  
protected:
  /**
   * This function returns the number of active children which is either the children named in the
   * optional "active" parameter or else all of the children underneath this Block
   */
  unsigned int n_activeChildren() const;

  /**
   * This function calles execute over all of the child blocks of the current Parser Block
   */
  void visitChildren(void (ParserBlock::*action)() = &ParserBlock::execute, bool visit_active_only=true);
  
  /************************************
   * Protected Data Members
   ************************************/
  std::string _reg_id;
  std::string _real_id;
  const GetPot & _input_file;

  /************************************
   * Private Data Members (use accessors)
   ************************************/
private:
  /**
   * The _block_params are those parameters which are valid for the currently parsed block but may not
   * necessarily be passed directly to the Factory constructor for this object type.  These should
   * be set in the constructor for each derived class and are an augmentation to the _class_params.
   */
  Parameters _block_params;

  /** The _class_params are those parameters which will be passed directly to the Factory constructor
   * objects directly.
   */
  Parameters _class_params;
  
  std::map<std::string, std::string> _doc_string;
  std::set<std::string> _required_params;
};

#endif //PARSERBLOCK_H
