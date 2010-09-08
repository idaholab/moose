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

#include "ParserBlockFactory.h"
#include "Parser.h"
#include "Moose.h"

// LibMesh includes
#include "InputParameters.h"


ParserBlockFactory *
ParserBlockFactory::instance()
{
  static ParserBlockFactory * instance;
  if(!instance)
    instance=new ParserBlockFactory;
    
  return instance;
}

ParserBlock *
ParserBlockFactory::add(const std::string & name, MooseSystem & moose_system, InputParameters params)
{
  ParserBlock * parser_block;
  std::string generic_identifier = ParserBlockFactory::instance()->isRegistered(name);
  
  parser_block = (*_name_to_build_pointer[generic_identifier])(name, moose_system, params);
  _active_parser_blocks.push_back(parser_block);

  return parser_block;
}

InputParameters
ParserBlockFactory::getValidParams(const std::string & name)
{
  std::string generic_identifier = ParserBlockFactory::instance()->isRegistered(name);

  if( _name_to_params_pointer.find(generic_identifier) == _name_to_params_pointer.end() )
    mooseError("No ParserBlock registered for \"" << name << "\"");
  else
    return _name_to_params_pointer[generic_identifier]();
}

ParserBlockIterator
ParserBlockFactory::activeParserBlocksBegin()
{
  return _active_parser_blocks.begin();
}

ParserBlockIterator
ParserBlockFactory::activeParserBlocksEnd()
{
  return _active_parser_blocks.end();
}

ParserBlockNamesIterator
ParserBlockFactory::registeredParserBlocksBegin()
{
  // Make sure the _registered_parserBlock_names are up to date
  _registered_parser_block_names.clear();
  _registered_parser_block_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, parserBlockParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_parser_block_names.push_back(i->first);
  }
  
  return _registered_parser_block_names.begin();
}

ParserBlockNamesIterator
ParserBlockFactory::registeredParserBlocksEnd()
{
  return _registered_parser_block_names.end();
}

std::string
ParserBlockFactory::isRegistered(const std::string & real_id)
{
  /**
   * This implementation assumes that wildcards can occur in the place of an entire token but not as part
   * of a token (i.e.  'Variables/* /InitialConditions' is valid but not 'Variables/Partial* /InitialConditions'.
   * Since maps are ordered, a reverse traversal through the registered list will always select a more
   * specific match before a wildcard match ('*' == char(42))
   */
  std::map<std::string, parserBlockParamsPtr>::reverse_iterator i;
  std::vector<std::string> real_elements, reg_elements;

  Parser::tokenize(real_id, real_elements);
  
  for (i=_name_to_params_pointer.rbegin(); i!=_name_to_params_pointer.rend(); ++i) 
  {
    std::string reg_id = i->first;
    if (reg_id == real_id) 
      return reg_id;
    
    reg_elements.clear();
    Parser::tokenize(reg_id, reg_elements);
    if (real_elements.size() == reg_elements.size())
    {
      bool keep_going = true;
      for (unsigned int j=0; keep_going && j<real_elements.size(); ++j)
      {
        if (real_elements[j] != reg_elements[j] && reg_elements[j] != std::string("*"))
          keep_going = false;
      }
      if (keep_going)
        return reg_id;
    }
  }
  
  return std::string("");
}


ParserBlockFactory:: ~ParserBlockFactory() 
{
  {
    std::map<std::string, parserBlockBuildPtr>:: iterator i;
    for(i=_name_to_build_pointer.begin(); i!=_name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, parserBlockParamsPtr>::iterator i;
    for(i=_name_to_params_pointer.begin(); i!=_name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
     
  {
        
    std::vector<ParserBlock *>::iterator i;
    for (i=_active_parser_blocks.begin(); i!=_active_parser_blocks.end(); ++i)
    {
      delete *i;
    }
    
  }
}

