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
  params.addPrivateParam<std::string>("built_by_action", "add_component");

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

std::vector<std::string>
Component::split(const std::string & rname)
{
  std::vector<std::string> splitted;
  Parser::tokenize(rname, splitted, 1, ":");

  std::string section_name("");
  for (unsigned int i = 0; i < splitted.size() - 1; i++)
  {
    if (i > 0)
      section_name.append(":");
    section_name.append(splitted[i]);
  }
  std::string prop_name = splitted[splitted.size() - 1];

  // construct the 2 element array with section and property name
  std::vector<std::string> result(2);
  result[0] = section_name;
  result[1] = prop_name;

  return result;
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
        // Testing
        // std::cout << "ComponentPostProcessor is added for " << _name << std::endl;
	/**/
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
	/**/
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

void
Component::connectObject(const std::string & rname, const std::string & mooseName)
{
  _rname_map[rname].push_back(mooseName);
}
