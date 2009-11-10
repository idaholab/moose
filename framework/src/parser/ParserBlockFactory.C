#include "ParserBlockFactory.h"
#include "Parser.h"

ParserBlockFactory *
ParserBlockFactory::instance()
{
  static ParserBlockFactory * instance;
  if(!instance)
    instance=new ParserBlockFactory;
    
  return instance;
}

ParserBlock *
ParserBlockFactory::add(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
{
  ParserBlock * parser_block;

  parser_block = (*name_to_build_pointer[reg_id])(reg_id, real_id, parent, parser_handle);
  active_parser_blocks.push_back(parser_block);

  return parser_block;
}

InputParameters
ParserBlockFactory::getValidParams(const std::string & name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
  {
    std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered ParserBlock "<<std::endl<<std::endl;
    mooseError("");
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
ParserBlockFactory::isRegistered(const std::string & real_id)
{
  /* This implementation assumes that wildcards can occur in the final position of the registered id.
   * Since maps are ordered, a reverse traversal through the registered list will always select a more
   * specific match before a wildcard match ('*' == char(42))
   */
  std::map<std::string, parserBlockParamsPtr>::reverse_iterator i;
  std::vector<std::string> real_elements, reg_elements;

  Parser::tokenize(real_id, real_elements);
  
  for (i=name_to_params_pointer.rbegin(); i!=name_to_params_pointer.rend(); ++i) 
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
    
    /* 
    else if (i->first[i->first.length()-1] == '*') 
    {
      size_t pos = real_id.rfind('/');
      // It's important to compare the trailing slashes to avoid premature matches when using
      // the wildcard behavior (pos+1).
      if (pos != std::string::npos && i->first.substr(0, i->first.rfind('/')+1) == real_id.substr(0, pos+1)) 
        return i->first;
     }
     */
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

