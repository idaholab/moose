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
  params.addPrivateParam<std::string>("built_by_action", "add_component");

  return params;
}

/*
 * Class used by Component class to map vector variables through friendly names
 * i.e. friendly name = inlet:K_loss; variableName = K_loss, position = 1
 */
RavenMapContainer::RavenMapContainer()
{
}
/*
 * CHANGE VARIABLENAME to ControllableName
 */
RavenMapContainer::RavenMapContainer(const std::string & controllableParName, unsigned int & position):
   _controllableParName(controllableParName),
   _position(position)
{
}
RavenMapContainer::~RavenMapContainer(){
}
const std::string &
RavenMapContainer::getControllableParName(){
   return _controllableParName;
}
unsigned int &
RavenMapContainer::getControllableParPosition(){
   return _position;
}

/*
 * Component implementation
 */
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
std::string
Component::genName(const std::string & prefix, const std::string & middle, const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << middle << suffix;
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
    _parent(parameters.have_parameter<Component *>("_parent") ? getParam<Component *>("_parent") : NULL),
    _sim(*getParam<Simulation *>("_sim")),
    _mesh(_sim.mesh()),
    _phys_mesh(_sim.physicalMesh()),
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
Component::checkEOSConsistency()
{
  InputParameters & pars = parameters();

  switch (_model_type)
  {
  case Model::EQ_MODEL_2:
  case Model::EQ_MODEL_3:
  case Model::EQ_MODEL_HEM:
    if (!pars.isParamValid("eos"))
      mooseError("Inconsistency in EOS detected in component '" << name() << "' - single phase model specified but single phase EOS not.");
    break;

  case Model::EQ_MODEL_7:
    if (!pars.isParamValid("eos_liquid") || !pars.isParamValid("eos_liquid"))
      mooseError("Inconsistency in EOS detected in component '" << name() << "' - two phase model specified but two phase EOS not.");
    break;
  }
}

void
Component::connectObject(const std::string & rname, const std::string & mooseName)
{
  _rname_map[rname].push_back(mooseName);
}

void
Component::createVectorControllableParMapping(const std::string & rname, const std::string & mooseName, unsigned int pos)
{
  _rvect_map[rname] = RavenMapContainer(mooseName, pos);
}
