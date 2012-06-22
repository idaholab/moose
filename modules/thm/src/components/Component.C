#include "Component.h"
#include "Simulation.h"

#include "ComponentPostProcessor.h"

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

std::string
Component::genName(const std::string & prefix, const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << ":" << suffix;
  return ss.str();
}

Component::Component(const std::string & name, InputParameters parameters) :
    R7Object(name, parameters),
    _id(comp_id++),
    _sim(*getParam<Simulation *>("_sim")),
    _mesh(_sim.mesh()),
    _model_type(_sim.getParam<Model::EModelType>("model_type")),

    _input_file_name(getParam<std::string>("physics_input_file"))
{

}

Component::~Component()
{
}

void
Component::init()
{
	/*
	{
		InputParameters params = validParams<ComponentPostProcessor>();
		params.set<Component*>("Component") = this;
		params.set<std::string>("output") = "none";
		params.set<std::string>("execute_on") = "residual";
		_sim.addPostprocessor("ComponentPostProcessor", genName("ComponentPPS_", _id, "_onResidual"), params);
	}
	{
		InputParameters params = validParams<ComponentPostProcessor>();
		params.set<Component*>("Component") = this;
		params.set<std::string>("output") = "none";
		params.set<std::string>("execute_on") = "timestep_begin";
		_sim.addPostprocessor("ComponentPostProcessor", genName("ComponentPPS_", _id, "_onTimestepBegin"), params);
	}
	{
		InputParameters params = validParams<ComponentPostProcessor>();
		params.set<Component*>("Component") = this;
		params.set<std::string>("output") = "none";
		params.set<std::string>("execute_on") = "timestep";
		_sim.addPostprocessor("ComponentPostProcessor", genName("ComponentPPS_", _id, "_onTimestepEnd"), params);
	}
	*/
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

unsigned int
Component::getNextBCId()
{
  unsigned int id = bc_ids++;
  return id;
}
