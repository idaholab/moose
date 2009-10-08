#ifndef PARSERBLOCK_H
#define PARSERBLOCK_H

#include "Kernel.h"

#include <vector>
#include <string>

//libMesh includes
#include "parameters.h"
#include "getpot.h"

/**
 * This class represents the base class for the various parser block data structures and the associated
 * member function which is responsible for setting up the appropriate data structures within MOOSE
 */
class ParserBlock 
{
public:
  /**
   * The registered id (reg_id) is a "path-like" identifier supporting an optional trailing
   * wildcard character '*'.  The real_id is the actual parsed path containing the real identifier
   * for this ParserBlock instance.  Finally a reference to the GetPot object is passed for
   * flexible extension.
   */
  ParserBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file);

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
   * This function calles execute over all of the child blocks of the current Parser Block
   */
  void visitChildren();

  /**
   * Child Parser Blocks (TODO: Probably should be hidden)
   */
  std::vector<ParserBlock *> _children;

  /** The _class_params are those parameters which will be passed directly to the Factory constructor
   * objects directly.
   */
  Parameters _class_params;

  /**
   * The _block_params are those parameters which are valid for the currently parsed block but may not
   * necessarily be passed directly to the Factory constructor for this object type.  These should
   * be set in the constructor for each derived class and are an augmentation to the _class_params.
   */
  Parameters _block_params;

protected:
  std::string _reg_id;
  std::string _real_id;
  const GetPot & _input_file;
};

#endif //PARSERBLOCK_H
