#include "Component.h"
#include "Simulation.h"

template<>
InputParameters validParams<Component>()
{
  InputParameters params = validParams<R7Object>();
  params.addPrivateParam<Simulation *>("_sim");
  return params;
}

static unsigned int comp_id = 0;

Component::Component(const std::string & name, InputParameters parameters) :
    R7Object(name, parameters),
    _id(comp_id++),
    _sim(*getParam<Simulation *>("_sim")),
    _mesh(_sim.mesh()),
    _problem(_sim.problem())
{
}

Component::~Component()
{
}

void
Component::init()
{
}


