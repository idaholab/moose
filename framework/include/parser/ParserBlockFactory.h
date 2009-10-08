#ifndef PARSERBLOCKFACTORY_H
#define PARSERBLOCKFACTORY_H

#include "ParserBlock.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

// LibMesh includes
#include <parameters.h>


/**
 * Typedef to make things easier.
 */
typedef ParserBlock * (*parserBlockBuildPtr)(const std::string & reg_id, const std::string & real_id, const GetPot & input_file);
/**
 * Typedef to make things easier.
 */
typedef Parameters (*parserBlockParamsPtr)();

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
ParserBlock * buildParserBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
{
  return new ParserBlockType(reg_id, real_id, input_file);
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

  ParserBlock * add(const std::string & red_id, const std::string & real_id, const GetPot & input_file);

  Parameters getValidParams(const std::string & name);
  
  ParserBlockIterator activeParserBlocksBegin();
  ParserBlockIterator activeParserBlocksEnd();

  ParserBlockNamesIterator registeredParserBlocksBegin();
  ParserBlockNamesIterator registeredParserBlocksEnd();

  std::string isRegistered(std::string & real_id);
  
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
