#include "ParserBlockFactory.h"

ParserBlockFactory *
ParserBlockFactory::instance()
{
  static ParserBlockFactory * instance;
  if(!instance)
    instance=new ParserBlockFactory;
    
  return instance;
}

ParserBlock *
ParserBlockFactory::add(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
{
  ParserBlock * parser_block;

  parser_block = (*name_to_build_pointer[reg_id])(reg_id, real_id, input_file);
  active_parser_blocks.push_back(parser_block);

  return parser_block;
}

Parameters
ParserBlockFactory::getValidParams(const std::string & name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
  {
    std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered ParserBlock "<<std::endl<<std::endl;
    libmesh_error();
  }
  return name_to_params_pointer[name]();
}

ParserBlockIterator
ParserBlockFactory::activeParserBlocksBegin()
{
  return active_parser_blocks.begin();
}

ParserBlockIterator
ParserBlockFactory::activeParserBlocksEnd()
{
  return active_parser_blocks.end();
}

ParserBlockNamesIterator
ParserBlockFactory::registeredParserBlocksBegin()
{
  // Make sure the _registered_parserBlock_names are up to date
  _registered_parser_block_names.clear();
  _registered_parser_block_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, parserBlockParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
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
ParserBlockFactory::isRegistered(std::string & real_id)
{
  std::map<std::string, parserBlockParamsPtr>::iterator i;

  for (i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i) 
  {
    if (i->first == real_id)
      return i->first;
    else if (i->first[i->first.length()-1] == '*') 
    {
      if (i->first.substr(0, i->first.find_last_of('/')) == real_id.substr(0, real_id.find_last_of('/')))
        return i->first;
    }
  }

  return std::string("");
}


ParserBlockFactory:: ~ParserBlockFactory() 
{
  {
    std::map<std::string, parserBlockBuildPtr>:: iterator i;
    for(i=name_to_build_pointer.begin(); i!=name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, parserBlockParamsPtr>::iterator i;
    for(i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
     
  {
        
    std::vector<ParserBlock *>::iterator i;
    for (i=active_parser_blocks.begin(); i!=active_parser_blocks.end(); ++i)
    {
      delete *i;
    }
    
  }
}

