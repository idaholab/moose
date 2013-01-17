#include "NodalAreaAux.h"
#include "NodalArea.h"

template<>
InputParameters validParams<NodalAreaAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("nodal_area_object", "The NodalArea UserObject to get values from");
  return params;
}

NodalAreaAux::NodalAreaAux(const std::string & name, InputParameters params) :
    AuxKernel(name, params),
    _nodal_area(getUserObject<NodalArea>("nodal_area_object"))
{
}

NodalAreaAux::~NodalAreaAux()
{
}

Real
NodalAreaAux::computeValue()
{
  return _nodal_area.nodalArea(_current_node->id());
}
