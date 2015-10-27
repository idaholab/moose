#include "Component.h"
#include "Simulation.h"
#include "MooseApp.h"

unsigned int Component::subdomain_ids = 0;
unsigned int Component::bc_ids = 0;

template<>
InputParameters validParams<Component>()
{
  InputParameters params = validParams<RELAP7Object>();
  params.addPrivateParam<Simulation *>("_sim");
  params.addPrivateParam<std::string>("built_by_action", "add_component");

  params.registerBase("Component");

  return params;
}

/*
 * Component implementation
 */
static unsigned int comp_id = 0;

void
Component::doBuildMesh()
{
  buildMesh();
}

std::string
Component::genName(const std::string & prefix, unsigned int id, const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << ":" << id;
  if (!suffix.empty())
    ss << ":" << suffix;
  return ss.str();
}

std::string
Component::genName(const std::string & prefix, unsigned int i, unsigned int j, const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << ":" << i << ":" << j;
  if (!suffix.empty())
    ss << ":" << suffix;
  return ss.str();
}

std::string
Component::genName(const std::string & prefix, const std::string & middle, const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << ":" << middle;
  if (!suffix.empty())
    ss << ":" << suffix;
  return ss.str();
}

std::vector<std::string>
Component::split(const std::string & rname)
{
  std::vector<std::string> splitted;
  MooseUtils::tokenize(rname, splitted, 1, ":");

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

Component::Component(const InputParameters & parameters) :
    RELAP7Object(parameters),
    _id(comp_id++),
    _parent(parameters.have_parameter<Component *>("_parent") ? getParam<Component *>("_parent") : NULL),
    _sim(*getParam<Simulation *>("_sim")),
    _factory(_app.getFactory()),
    _mesh(_sim.mesh()),
    _phys_mesh(_sim.physicalMesh()),
    _zero(_sim._zero)
{
}

Component::~Component()
{
}

void
Component::init()
{
}

unsigned int
Component::getNextSubdomainId()
{
  unsigned int sd_id = subdomain_ids++;
  _subdomains.push_back(sd_id);
  if (_parent)
    _parent->_subdomains.push_back(sd_id);
  return sd_id;
}

unsigned int
Component::getNextBCId()
{
  unsigned int id = bc_ids++;
  return id;
}

void
Component::aliasParam(const std::string & rname, const std::string & name, Component * comp/* = NULL*/)
{
  if (comp == NULL)
    _param_alias_map[rname] = std::pair<Component *, std::string>(this, name);
  else
    _param_alias_map[rname] = std::pair<Component *, std::string>(comp, name);
}

void
Component::aliasVectorParam(const std::string & rname, const std::string & name, unsigned int pos, Component * /*comp = NULL*/)
{
  createVectorControllableParMapping(rname, name, pos);
}

void
Component::connectObject(const InputParameters & params, const std::string & rname, const std::string & mooseName, const std::string & name)
{
  ControlLogicNameEntry rne(params.get<std::string>("_moose_base") + "::" + mooseName, name);
  if (_parent != NULL)
    _parent->_rname_map[rname][name].push_back(rne);
  else
    _rname_map[rname][name].push_back(rne);
}

void
Component::connectObject(const InputParameters & params, const std::string & rname, const std::string & mooseName, const std::string & name, const std::string & par_name)
{
  ControlLogicNameEntry rne(params.get<std::string>("_moose_base") + "::" + mooseName, par_name);
  if (_parent != NULL)
    _parent->_rname_map[rname][name].push_back(rne);
  else
    _rname_map[rname][name].push_back(rne);
}


void
Component::createVectorControllableParMapping(const std::string & rname, const std::string & mooseName, unsigned int pos)
{
  ControlLogicMapContainer mc(mooseName, pos);
  _rvect_map.insert(std::pair<std::string, ControlLogicMapContainer>(rname, mc));
}
