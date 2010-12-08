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

#ifndef PARSERBLOCKFACTORY_H
#define PARSERBLOCKFACTORY_H

#include "ParserBlock.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

/**
 * Typedef to make things easier.
 */
typedef ParserBlock * (*parserBlockBuildPtr)(const std::string & name, InputParameters params);
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
ParserBlock * buildParserBlock(const std::string & name, InputParameters params)
{
  return new ParserBlockType(name, params);
}

/**
 * Responsible for building ParserBlocks on demand and storing them for retrieval
 */
class ParserBlockFactory
{
public:
  static ParserBlockFactory * instance();

  template<typename ParserBlockType> 
  void registerParserBlock(const std::string & name)
  {
    _name_to_build_pointer[name]=&buildParserBlock<ParserBlockType>;
    _name_to_params_pointer[name]=&validParams<ParserBlockType>;
  }

  ParserBlock * add(const std::string & name, InputParameters params);

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
  
  std::map<std::string, parserBlockBuildPtr> _name_to_build_pointer;
  std::map<std::string, parserBlockParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_parser_block_names;
  std::vector<ParserBlock *> _active_parser_blocks;

};

#endif //PARSERBLOCKFACTORY_H
