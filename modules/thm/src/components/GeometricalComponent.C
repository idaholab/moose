#include "GeometricalComponent.h"
#include "Conversion.h"
#include "ConstantFunction.h"

template<>
InputParameters validParams<GeometricalComponent>()
{
  InputParameters params = validParams<Component>();

  params.addRequiredParam<Point>("position", "Origin (start) of the pipe");
  params.addParam<RealVectorValue>("offset", RealVectorValue(), "Offset of the origin for mesh generation");
  params.addRequiredParam<RealVectorValue>("orientation", "Orientation vector of the pipe");
  params.addParam<Real>("rotation", 0., "Rotation of the component (in degrees)");

  params.addRequiredParam<Real>("length", "The length of the geometric component along the main axis");

  params.addParam<unsigned int>("n_elems", 0, "The number of elements along the main axis");
  std::vector<Real> default_node_locations (1, 0.0);
  params.addParam<std::vector<Real> >("node_locations", default_node_locations, "Node locations along the main axis.");

  return params;
}

GeometricalComponent::GeometricalComponent(const InputParameters & parameters) :
    Component(parameters),
    _position(getParam<Point>("position")),
    _offset(getParam<RealVectorValue>("offset")),
    _dir(getParam<RealVectorValue>("orientation")),
    _rotation(getParam<Real>("rotation")),
    _2nd_order_mesh(_sim.getParam<bool>("2nd_order_mesh")),
    _length(getParam<Real>("length")),
    _n_elems(getParam<unsigned int>("n_elems")),
    _node_locations(getParam<std::vector<Real> >("node_locations"))
{
}

GeometricalComponent::~GeometricalComponent()
{
}

void
GeometricalComponent::doBuildMesh()
{
  processNodeLocations();
  _first_node_id = _mesh.nNodes();
  Component::doBuildMesh();
  _last_node_id = _mesh.nNodes();

  // shift the component in z-direction so they do not overlap
  // NOTE: we are using 1D elements, so shifting each component by 1 is ok for now
  // When using 2D or even 3D meshes, this has to be way smarter
  for (unsigned int node_id = _first_node_id; node_id < _last_node_id; node_id++)
    _mesh.nodeRef(node_id)(2) += id();
}

void
GeometricalComponent::displaceMesh()
{
  Real rotation = M_PI * _rotation / 180.;
  RealVectorValue z_offset(0, 0, id());

  // rows of rotation matrix to rotate about x-axis
  RealVectorValue Rx_x(1, 0,              0);
  RealVectorValue Rx_y(0, cos(rotation), -sin(rotation));
  RealVectorValue Rx_z(0, sin(rotation),  cos(rotation));
  RealTensorValue Rx(Rx_x, Rx_y, Rx_z);

  // figure out the rotation
  Real r = _dir.norm();
  Real theta = acos(_dir(2) / r);
  Real aphi = atan2(_dir(1), _dir(0));
  // rows of transformation matrix
  RealVectorValue x( cos(-aphi)*cos(M_PI/2-theta), sin(-aphi), -cos(-aphi)*sin(M_PI/2-theta));
  RealVectorValue y(-sin(-aphi)*cos(M_PI/2-theta), cos(-aphi),  sin(-aphi)*sin(M_PI/2-theta));
  RealVectorValue z( sin(M_PI/2-theta),            0.0,         cos(M_PI/2-theta));
  RealTensorValue R(x, y, z);

  for (unsigned int node_id = _first_node_id; node_id < _last_node_id; ++node_id)
  {
    Node & current_node = _phys_mesh->nodeRef(node_id);
    RealVectorValue p(current_node(0), current_node(1), current_node(2));
    // move to the origin, rotate about x-axis, transform to follow the direction and move to its position
    current_node = R * (Rx * (p + _offset - z_offset)) + _position;
  }
}

const std::vector<RELAP7::Connection> &
GeometricalComponent::getConnections(RELAP7::EEndType id) const
{
  std::map<RELAP7::EEndType, std::vector<RELAP7::Connection> >::const_iterator it = _connections.find(id);
  if (it != _connections.end())
    return it->second;
  else
    mooseError(name() << ": No end of this type available (" << id << ").");
}

void
GeometricalComponent::processNodeLocations()
{
  bool specified_n_elems = _n_elems != 0;
  unsigned int n_nodes = _node_locations.size();
  bool specified_node_locations = n_nodes > 1;

  if (specified_n_elems)
  {
    if (specified_node_locations)
    {
      unsigned int expected_n_elems = _2nd_order_mesh ? (n_nodes - 1) / 2 : n_nodes - 1;
      if (expected_n_elems != _n_elems) {
        mooseError(name() << ": \"n_elems\" and \"node_locations\" do not match.");
      }
    }
    else
    {
      _node_locations = std::vector<Real>(_n_elems + 1, 0.0);
      Real dx = _length / _n_elems;
      for (int i = 0; i < _n_elems - 1; ++i)
      {
        _node_locations[i + 1] = _node_locations[i] + dx;
      }
      _node_locations[_n_elems] = _length;
    }
  }
  else
  {
    if (specified_node_locations)
    {
      // remove duplicates and order in an increassing manner
      std::set<Real> unq_ord_node_lcns( _node_locations.begin(), _node_locations.end());
      _node_locations.assign(unq_ord_node_lcns.begin(), unq_ord_node_lcns.end());

      // removes locations not in the component
      std::vector<Real> new_nodes;
      new_nodes.reserve(_node_locations.size() + 2);
      for (auto node_location : _node_locations)
      {
        if ((node_location >= 0.0) && (node_location <= _length))
        {
          new_nodes.push_back(node_location);
        }
      }

      // add entry boundary node if not explicitly specified
      if (new_nodes[0] != 0.0)
      {
        new_nodes.insert(new_nodes.begin(), 0.0);
      }

      // add exit boundary node if not explicitly specified
      if (new_nodes[new_nodes.size() - 1] != _length)
      {
        new_nodes.push_back(_length);
      }

      _node_locations = new_nodes;

      // determine number of elements
      n_nodes = _node_locations.size();
      _n_elems = _2nd_order_mesh ? (n_nodes - 1) / 2 : n_nodes - 1;
    }
    else
    {
      mooseError(name() << ": \"n_elems\" or \"node_locations\" must be specified.");
    }
  }

  if (_2nd_order_mesh)
  {
    unsigned int new_n_nodes = (2 * _n_elems) + 1;
    std::vector<Real> new_nodes(new_n_nodes, 0.0);
    unsigned int new_node_indx = 0;

    for (int i = 0; i < _n_elems; ++i)
    {
      new_nodes[new_node_indx] = _node_locations[i];
      ++new_node_indx;
      new_nodes[new_node_indx] = 0.5 * (_node_locations[i] + _node_locations[i+1]);
      ++new_node_indx;
    }
    new_nodes[new_node_indx] = _node_locations[_n_elems];
    _node_locations = new_nodes;
  }
}

const FunctionName &
GeometricalComponent::getVariableFn(const FunctionName & fn_param_name)
{
  const FunctionName & fn_name = getParam<FunctionName>(fn_param_name);
  const Function & fn = _sim.getFunction(fn_name);

  if (dynamic_cast<const ConstantFunction *>(&fn) != NULL)
  {
    connectObject(fn.parameters(), "", fn_name, fn_param_name, "value");
  }

  return fn_name;
}
