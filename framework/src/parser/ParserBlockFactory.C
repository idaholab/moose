#include "ParserBlockFactory.h"
#include "Parser.h"


ParserBlockFactory *ParserBlockFactory::instance()
{
  static ParserBlockFactory *instance;
  if (!instance)
    instance = new ParserBlockFactory;

  return instance;
}

ParserBlockFactory:: ~ParserBlockFactory()
{
  for (std::vector<ParserBlock *>::iterator i=_active_parser_blocks.begin(); i!=_active_parser_blocks.end(); ++i)
    delete *i;
}

ParserBlock *
ParserBlockFactory::add(const std::string & name, InputParameters params)
{
  ParserBlock * parser_block;
  std::string generic_identifier = ParserBlockFactory::instance()->isRegistered(name);

  parser_block = (*_name_to_build_pointer[generic_identifier])(name, params);
  _active_parser_blocks.push_back(parser_block);

  return parser_block;
}

InputParameters
ParserBlockFactory::getValidParams(const std::string & name)
{
  std::string generic_identifier = ParserBlockFactory::instance()->isRegistered(name);

  if( _name_to_params_pointer.find(generic_identifier) == _name_to_params_pointer.end() )
    mooseError(std::string("A '") + name + "' is not a registered parser block\n\n");

  return _name_to_params_pointer[generic_identifier]();
}

std::string
ParserBlockFactory::isRegistered(const std::string & real_id)
{
  /**
   * This implementation assumes that wildcards can occur in the place of an entire token but not as part
   * of a token (i.e.  'Variables/ * /InitialConditions' is valid but not 'Variables/Partial* /InitialConditions'.
   * Since maps are ordered, a reverse traversal through the registered list will always select a more
   * specific match before a wildcard match ('*' == char(42))
   */
  std::map<std::string, paramsParserBlockPtr>::reverse_iterator i;
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

