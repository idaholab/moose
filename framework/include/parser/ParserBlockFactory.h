#ifndef PARSERBLOCKFACTORY_H_
#define PARSERBLOCKFACTORY_H_

#include <vector>

#include "Moose.h"
#include "ParserBlock.h"
#include "InputParameters.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerNamedParserBlock(tplt, name)      ParserBlockFactory::instance()->reg<tplt>(name)


/**
 * Typedef for function to build objects
 */
typedef ParserBlock * (*buildParserBlockPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsParserBlockPtr)();


/**
 * Build an object of type T
 */
template<class T>
ParserBlock * buildParserBlock(const std::string & name, InputParameters parameters)
{
  return new T(name, parameters);
}


/**
 * Generic factory class for build all sorts of objects
 */
class ParserBlockFactory
{
public:
  static ParserBlockFactory *instance();

  virtual ~ParserBlockFactory();

  template<typename T>
  void reg(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildParserBlock<T>;
      _name_to_params_pointer[name] = &validParams<T>;
    }
    else
      mooseError("Parser block '" + name + "' already registered.");
  }

  ParserBlock * add(const std::string & name, InputParameters params);

  InputParameters getValidParams(const std::string & name);

  std::string isRegistered(const std::string & real_id);

protected:
  std::map<std::string, buildParserBlockPtr>  _name_to_build_pointer;
  std::map<std::string, paramsParserBlockPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_parser_block_names;
  std::vector<ParserBlock *> _active_parser_blocks;
};

#endif /* PARSERBLOCKFACTORY_H_ */
