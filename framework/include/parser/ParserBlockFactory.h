#ifndef PARSERBLOCKFACTORY_H
#define PARSERBLOCKFACTORY_H

#include "ParserBlock.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

// LibMesh includes
#include "InputParameters.h"


/**
 * Typedef to make things easier.
 */
typedef ParserBlock * (*parserBlockBuildPtr)(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);
/**
 * Typedef to make things easier.
 */
typedef InputParameters (*parserBlockParamsPtr)();

/**
 * Typedef to hide implementation details
 */
typedef std::vector<ParserBlock *>::iterator ParserBlockIterator;

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator ParserBlockNamesIterator;

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename ParserBlockType>
ParserBlock * buildParserBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
{
  return new ParserBlockType(reg_id, real_id, parent, parser_handle);
}

/**
 * Responsible for building ParserBlocks on demand and storing them for retrieval
 */
class ParserBlockFactory
{
public:
  static ParserBlockFactory * instance();

  template<typename ParserBlockType> 
  void registerParserBlock(std::string name)
  {
    name_to_build_pointer[name]=&buildParserBlock<ParserBlockType>;
    name_to_params_pointer[name]=&valid_params<ParserBlockType>;
  }

  ParserBlock * add(const std::string & red_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);

  InputParameters getValidParams(const std::string & name);
  
  ParserBlockIterator activeParserBlocksBegin();
  ParserBlockIterator activeParserBlocksEnd();

  ParserBlockNamesIterator registeredParserBlocksBegin();
  ParserBlockNamesIterator registeredParserBlocksEnd();

  std::string isRegistered(const std::string & real_id);
  
private:
  ParserBlockFactory()
    {}

  virtual ~ParserBlockFactory();
  
  std::map<std::string, parserBlockBuildPtr> name_to_build_pointer;
  std::map<std::string, parserBlockParamsPtr> name_to_params_pointer;

  std::vector<std::string> _registered_parser_block_names;
  std::vector<ParserBlock *> active_parser_blocks;

};

#endif //PARSERBLOCKFACTORY_H
