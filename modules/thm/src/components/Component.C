#include "Component.h"

template<>
InputParameters validParams<Component>()
{
  InputParameters params = validParams<R7Object>();
  params.addPrivateParam<MooseMesh *>("_mesh");
  return params;
}

static unsigned int comp_id = 0;

Component::Component(const std::string & name, InputParameters parameters) :
    R7Object(name, parameters),
    _id(comp_id++),
    _mesh(*getParam<MooseMesh *>("_mesh"))
{
}

Component::~Component()
{
}

void
Component::init()
{
}


