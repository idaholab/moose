#include "Component.h"
#include "Simulation.h"

template<>
InputParameters validParams<Component>()
{
  InputParameters params = validParams<R7Object>();
  params.addPrivateParam<Simulation *>("_sim");

  params.addParam<std::string>("physics_input_file", "Input file with physics");

  return params;
}

static unsigned int comp_id = 0;

Component::Component(const std::string & name, InputParameters parameters) :
    R7Object(name, parameters),
    _id(comp_id++),
    _sim(*getParam<Simulation *>("_sim")),
    _mesh(_sim.mesh()),
    _problem(_sim.problem()),

    _input_file_name(getParam<std::string>("physics_input_file")),
    _parser(Moose::syntax)
{
}

Component::~Component()
{
}

void
Component::init()
{
}

void
Component::parseInput()
{
  // setup parser
  _parser._mesh = &_mesh;
  _parser._problem = &_problem;

  _parser.parse(_input_file_name);
}
