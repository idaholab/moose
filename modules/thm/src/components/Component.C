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
  return splitted;
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

template<typename T>
const T &
Component::getRParam(const std::string & param_name)
{
  std::vector<std::string> s = split(param_name);

  const std::vector<std::string> & names = getMooseObjectsByName(s[0]);
  for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
  {
    std::string nm = *it;
    const std::vector<Material *> & mats = _sim.feproblem().getMaterialsByName(nm, 0);
    if (mats.size() != 1)
      mooseError("No or multiple objects with name '" + nm + "'.");

    Material * mat = mats[0];
    if (mat->parameters().have_parameter<T>(s[1]))
      return mat->parameters().get<T>(s[1]);
  }
}

template<typename T>
void
Component::setRParam(const std::string & param_name, const T & value)
{
  std::vector<std::string> s = split(param_name);

  const std::vector<std::string> & names = getMooseObjectsByName(s[0]);
  for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
  {
    std::string nm = *it;
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      const std::vector<Material *> & mats = _sim.feproblem().getMaterialsByName(nm, tid);
      if (mats.size() != 1)
        mooseError("No or multiple objects with name '" + nm + "'.");

      Material * mat = mats[0];
      if (mat->parameters().have_parameter<T>(s[1]))
        mat->parameters().set<T>(s[1]) = value;
    }
  }
}

void
Component::connectObject(const std::string & rname, const std::string & mooseName)
{
  _rname_map[rname].push_back(mooseName);
}
