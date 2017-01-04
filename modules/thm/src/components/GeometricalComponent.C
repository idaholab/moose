#include "GeometricalComponent.h"
#include "Conversion.h"
#include "ConstantFunction.h"
#include "R7Conversion.h"

template<>
InputParameters validParams<GeometricalComponent>()
{
  InputParameters params = validParams<Component>();

  params.addRequiredParam<Point>("position", "Origin (start) of the pipe");
  params.addParam<RealVectorValue>("offset", RealVectorValue(), "Offset of the origin for mesh generation");
  params.addRequiredParam<RealVectorValue>("orientation", "Orientation vector of the pipe");
  params.addParam<Real>("rotation", 0., "Rotation of the component (in degrees)");

  params.addParam<Real>("length", "The length of the geometric component along the main axis");
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
      std::set<Real> unique_nodes;
      std::unordered_set<Real> duplicate_nodes;
      for (auto node_location : _node_locations)
      {
        if (unique_nodes.find(node_location) != unique_nodes.end())
        {
          duplicate_nodes.emplace(node_location);
        }
        else
        {
          unique_nodes.emplace(node_location);
        }
      }

      // warn user if duplicate nodes were found
      if (duplicate_nodes.size() > 0)
      {
        std::string dup_node_str = RELAP7::containerToString(duplicate_nodes);
        mooseWarning(name() << ": Duplicate nodes being ignored (" << dup_node_str << ").");
      }

      // removes locations not in the component
      std::vector<Real> actual_nodes;
      actual_nodes.reserve(_node_locations.size() + 2);
      std::vector<Real> invalid_nodes;
      for (auto node_location : unique_nodes)
      {
        if ((node_location >= 0.0) && (node_location <= _length))
        {
          actual_nodes.push_back(node_location);
        }
        else
        {
          invalid_nodes.push_back(node_location);
        }
      }

      // warn user if invalid nodes were found
      if (invalid_nodes.size() > 0)
      {
        std::string inv_node_str = RELAP7::containerToString(invalid_nodes);
        mooseWarning(name() << ": Invalid nodes being ignored (" << inv_node_str << ").");
      }

      // add entry boundary node if not explicitly specified
      if (actual_nodes[0] != 0.0)
      {
        actual_nodes.insert(actual_nodes.begin(), 0.0);
      }

      // add exit boundary node if not explicitly specified
      if (actual_nodes[actual_nodes.size() - 1] != _length)
      {
        actual_nodes.push_back(_length);
      }

      _node_locations = actual_nodes;

      // determine number of elements
      n_nodes = _node_locations.size();
      _n_elems = _2nd_order_mesh ? (n_nodes - 1) / 2 : n_nodes - 1;
    }
    else
    {
      mooseError(name() << ": \"n_elems\" or \"node_locations\" must be specified.");
    }
  }

  if (_2nd_order_mesh) generateIntermediateNodes();
}

void
GeometricalComponent::generateIntermediateNodes()
{
  unsigned int actual_n_nodes = (2 * _n_elems) + 1;
  std::vector<Real> actual_nodes(actual_n_nodes, 0.0);
  unsigned int new_node_indx = 0;

  for (int i = 0; i < _n_elems; ++i)
  {
    actual_nodes[new_node_indx] = _node_locations[i];
    ++new_node_indx;
    actual_nodes[new_node_indx] = 0.5 * (_node_locations[i] + _node_locations[i+1]);
    ++new_node_indx;
  }
  actual_nodes[new_node_indx] = _node_locations[_n_elems];
  _node_locations = actual_nodes;
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
