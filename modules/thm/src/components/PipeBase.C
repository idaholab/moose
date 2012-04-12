#include "PipeBase.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "R7Conversion.h"
#include "OneDEnergyWallHeating.h"

#include "edge_edge2.h"
#include "fe_type.h"

// physics
#include "Diffusion.h"

const std::string PipeBase::_type("pipe");


template<>
InputParameters validParams<PipeBase>()
{
  InputParameters params = validParams<Component>();
  //Input parameters [NO] default values should be given.
  params.addRequiredParam<std::vector<Real> >("position", "Origin (start) of the pipe");
  params.addRequiredParam<std::vector<Real> >("orientation", "Orientation vector of the pipe");
  params.addRequiredParam<Real>("length", "Length of the pipe");
  params.addRequiredParam<unsigned int>("n_elems", "number of element in this pipe");
  params.addRequiredParam<Real>("A", "Area of the pipe");

  //Input parameters default values could be given.
  params.addParam<Real>("aw", 0.0, "Heating surface density");
  params.addParam<Real>("f", 0.0, "friction");
  params.addParam<Real>("Hw", 0.0, "Convective heat transfer coefficient");
  params.addParam<Real>("Tw", 400, "Wall temperature");

  return params;
}


PipeBase::PipeBase(const std::string & name, InputParameters params) :
    Component(name, params),
    Model(params),
    _position(toPoint(getParam<std::vector<Real> >("position"))),
    _length(getParam<Real>("length")),
    _n_elems(getParam<unsigned int>("n_elems")),
    _A(getParam<Real>("A")),
    _aw(getParam<Real>("aw")),
    _f(getParam<Real>("f")),
    _Hw(getParam<Real>("Hw")),
    _Tw(getParam<Real>("Tw"))
{
  const std::vector<Real> & dir = getParam<std::vector<Real> >("orientation");
  _dir = VectorValue<Real>(dir[0], dir[1], dir[2]);
}

PipeBase::~PipeBase()
{
}

Node *
PipeBase::getBoundaryNode(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, Node *>::iterator it = _bnd_nodes.find(id);
  if (it != _bnd_nodes.end())
    return it->second;
  else
    return NULL;
}

unsigned int
PipeBase::getBoundaryId(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, unsigned int>::iterator it = _bnd_ids.find(id);
  if (it != _bnd_ids.end())
    return it->second;
  else
    mooseError("PipeBase " << name() << " does not have this type of end defined.");
}

void
PipeBase::buildMesh()
{
  // points
  Real delta_t = _length / _n_elems;
  Point p(0, 0, 0);                      // origin
  for (unsigned int i = 0; i <= _n_elems; i++)
  {
    const Node * nd = _mesh._mesh.add_point(p);
    node_ids.push_back(nd->id());
    p(0) += delta_t;
  }

  // elems
  _subdomain_id = getNextSubdomainId();
  unsigned int bc_id_in = getNextBCId();
  unsigned int bc_id_out = getNextBCId();
  for (unsigned int i = 0; i < _n_elems; i++)
  {
    Elem * elem = _mesh._mesh.add_elem (new Edge2);
    elem->subdomain_id() = _subdomain_id;
    elem->set_node(0) = _mesh._mesh.node_ptr(node_ids[i]);
    elem->set_node(1) = _mesh._mesh.node_ptr(node_ids[i+1]);

    // BCs
    if (i == 0)
    {
      _bnd_ids[RELAP7::IN] = bc_id_in;
      _mesh._mesh.boundary_info->add_side(elem, 0, bc_id_in);
      _bnd_nodes[RELAP7::IN] = elem->get_node(0);                // first 0 is local bnd_id that Joints will use for connecting
    }
    if (i == (_n_elems - 1))
    {
      _bnd_ids[RELAP7::OUT] = bc_id_out;
      _mesh._mesh.boundary_info->add_side(elem, 1, bc_id_out);
      _bnd_nodes[RELAP7::OUT] = elem->get_node(1);               // first 1 is local bnd_id that Joints will use for connecting
    }
 }
}

void
PipeBase::addVariables()
{
  Model::addVariables(_subdomain_id);
}

std::vector<unsigned int>
PipeBase::getIDs(std::string /*piece*/)
{
  mooseError("Not implemented yet");
  return std::vector<unsigned int>();
}

std::string
PipeBase::variableName(std::string /*piece*/)
{
  mooseError("Not implemented yet");
  return std::string();
}
