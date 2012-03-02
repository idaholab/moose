#include "Component.h"
#include "Simulation.h"

unsigned int Component::subdomain_ids = 0;
unsigned int Component::bc_ids = 0;

template<>
InputParameters validParams<Component>()
{
  InputParameters params = validParams<R7Object>();
  params.addPrivateParam<Simulation *>("_sim");

  params.addParam<std::string>("physics_input_file", "Input file with physics");

  return params;
}

static unsigned int comp_id = 0;

std::string
Component::genName(const std::string & prefix, unsigned int id, const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << id << suffix;
  return ss.str();
}

Component::Component(const std::string & name, InputParameters parameters) :
    R7Object(name, parameters),
    _id(comp_id++),
    _sim(*getParam<Simulation *>("_sim")),
    _mesh(_sim.mesh()),

    _input_file_name(getParam<std::string>("physics_input_file"))
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
  if (!_input_file_name.empty())
  {
    // TODO: parse local input file
  }
}

unsigned int
Component::getNextSubdomainId()
{
  unsigned int sd_id = subdomain_ids++;
  _subdomains.push_back(sd_id);
  return sd_id;
}
