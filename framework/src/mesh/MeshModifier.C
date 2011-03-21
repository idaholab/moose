#include "MeshModifier.h"

template<>
InputParameters validParams<MeshModifier>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<std::string>("built_by_action", "add_mesh_modifier");
  return params;
}

MeshModifier::MeshModifier(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters)
{
}

