#include "Action.h"

template<>
InputParameters validParams<Action>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  // Add the "active" parameter to all blocks to support selective child visitation (turn blocks on and off without comments)
  params.addParam<std::vector<std::string> >("active", blocks, "If specified only the blocks named will be visited and made active");
  params.addParam<std::string>("type", "A string representing the object type that this ParserBlock will hold if applicable");
  params.addPrivateParam<ParserBlock *>("parent");
  params.addPrivateParam<Parser *>("parser_handle");
 
  return params;
}


Action::Action(const std::string & name, InputParameters params) :
    _name(name),
    _action(params.get<std::string>("action")),
    _params(params)
{
}

