#include "Component.h"
#include "Simulation.h"
#include "MooseApp.h"

unsigned int Component::subdomain_ids = 0;
unsigned int Component::bc_ids = 0;

template <>
InputParameters
validParams<Component>()
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
Component::genName(const std::string & prefix,
                   unsigned int i,
                   unsigned int j,
                   const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << ":" << i << ":" << j;
  if (!suffix.empty())
    ss << ":" << suffix;
  return ss.str();
}

std::string
Component::genName(const std::string & prefix,
                   const std::string & middle,
                   const std::string & suffix)
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

Component::Component(const InputParameters & parameters)
  : RELAP7Object(parameters),
    _id(comp_id++),
    _parent(parameters.have_parameter<Component *>("_parent") ? getParam<Component *>("_parent")
                                                              : NULL),
    _sim(*getParam<Simulation *>("_sim")),
    _app(dynamic_cast<RELAP7App &>(MooseObject::_app)),
    _factory(_app.getFactory()),
    _mesh(_sim.mesh()),
    _phys_mesh(_sim.physicalMesh()),
    _zero(_sim._zero)
{
}

void
Component::init()
{
}

void
Component::check()
{
}

unsigned int
Component::getNextSubdomainId()
{
  unsigned int sd_id = subdomain_ids++;
  _subdomains.push_back(sd_id);
  _coord_sys.push_back(Moose::COORD_XYZ);
  if (_parent)
  {
    _parent->_subdomains.push_back(sd_id);
    _parent->_coord_sys.push_back(Moose::COORD_XYZ);
  }
  return sd_id;
}

unsigned int
Component::getNextBCId()
{
  unsigned int id = bc_ids++;
  return id;
}

void
Component::setSubdomainCoordSystem(unsigned int block_id, Moose::CoordinateSystemType coord_type)
{
  if (_parent)
    _parent->setSubdomainCoordSystem(block_id, coord_type);

  for (unsigned int i = 0; i < _subdomains.size(); i++)
  {
    if (_subdomains[i] == block_id)
    {
      _coord_sys[i] = coord_type;
      return;
    }
  }

  mooseError(name(),
             ": Trying to set coordinate system ",
             coord_type,
             " on a block id '",
             block_id,
             "',"
             " but this component does not have such a block.");
}

void
Component::connectObject(const InputParameters & params,
                         const std::string & rname,
                         const std::string & mooseName,
                         const std::string & name)
{
  connectObject(params, rname, mooseName, name, name);
}

void
Component::connectObject(const InputParameters & params,
                         const std::string & /*rname*/,
                         const std::string & mooseName,
                         const std::string & name,
                         const std::string & par_name)
{
  MooseObjectParameterName alias("component/" + this->name() + "/" + name);
  MooseObjectParameterName par_value(
      MooseObjectName(params.get<std::string>("_moose_base"), mooseName), par_name);
  _app.getInputParameterWarehouse().addControllableParameterConnection(alias, par_value);
}
