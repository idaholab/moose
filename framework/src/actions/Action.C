#include "Action.h"

template<>
InputParameters validParams<Action>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  // Add the "active" parameter to all blocks to support selective child visitation (turn blocks on and off without comments)
  params.addParam<std::vector<std::string> >("active", blocks, "If specified only the blocks named will be visited and made active");
  params.addPrivateParam<Parser *>("parser_handle");
  return params;
}


Action::Action(const std::string & name, InputParameters params) :
    MooseObject(name, params),
    _action(getParam<std::string>("action")),
    _parser_handle(*getParam<Parser *>("parser_handle"))
{
}

std::string
Action::getShortName() const
{
  return _name.substr(_name.find_last_of('/') != std::string::npos ? _name.find_last_of('/') + 1 : 0);
}
