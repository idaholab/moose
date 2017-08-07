#include "GeometricalComponent.h"
#include "Conversion.h"
#include "ConstantFunction.h"

template <>
InputParameters
validParams<GeometricalComponent>()
{
  InputParameters params = validParams<Component>();

  params.addRequiredParam<Point>("position", "Origin (start) of the pipe");
  params.addParam<RealVectorValue>(
      "offset", RealVectorValue(), "Offset of the origin for mesh generation");
  params.addRequiredParam<RealVectorValue>("orientation", "Orientation vector of the pipe");
  params.addParam<Real>("rotation", 0., "Rotation of the component (in degrees)");
  params.addRequiredParam<std::vector<Real>>(
      "length", "The lengths of the subsections of the geometric component along the main axis");
  params.addRequiredParam<std::vector<unsigned int>>(
      "n_elems", "The number of elements in each subsection along the main axis");

  return params;
}

GeometricalComponent::GeometricalComponent(const InputParameters & parameters)
  : Component(parameters),
    _position(getParam<Point>("position")),
    _offset(getParam<RealVectorValue>("offset")),
    _dir(getParam<RealVectorValue>("orientation")),
    _rotation(getParam<Real>("rotation")),
    _2nd_order_mesh(_sim.getParam<bool>("2nd_order_mesh")),
    _lengths(getParam<std::vector<Real>>("length")),
    _n_elems(getParam<std::vector<unsigned int>>("n_elems"))
{
  validateNSectionsConsistent(_lengths.size(), _n_elems.size());
  _n_sections = _lengths.size();
  _length = std::accumulate(_lengths.begin(), _lengths.end(), 0.0);
  _n_elem = std::accumulate(_n_elems.begin(), _n_elems.end(), 0);
  _n_nodes = computeNumberOfNodes(_n_elem);
}

void
GeometricalComponent::validateNSectionsConsistent(int n_lengths, int n_n_elems)
{
  bool specified_lengths = n_lengths > 0;
  bool specified_n_elems = n_n_elems > 0;
  bool agreeing_inputs = n_lengths == n_n_elems;
  bool valid_inputs = specified_n_elems && specified_lengths && agreeing_inputs;

  if (!valid_inputs)
  {
    if (!agreeing_inputs)
      logError("The number of entries in the parameter 'length' does not equal the number of "
               "entries in the parameter 'n_elems'.");

    if (!specified_lengths)
      logError("There are zero entries for the parameter 'length'.");

    if (!specified_n_elems)
      logError("There are zero entries for the parameter 'n_elems'.");
  }
}

unsigned int
GeometricalComponent::computeNumberOfNodes(unsigned int n_elems)
{
  return _2nd_order_mesh ? (2 * n_elems) + 1 : n_elems + 1;
}

void
GeometricalComponent::setupMesh()
{
  generateNodeLocations();
  _first_node_id = _mesh.nNodes();
  buildMesh();
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
  RealVectorValue Rx_x(1, 0, 0);
  RealVectorValue Rx_y(0, cos(rotation), -sin(rotation));
  RealVectorValue Rx_z(0, sin(rotation), cos(rotation));
  RealTensorValue Rx(Rx_x, Rx_y, Rx_z);

  // figure out the rotation
  Real r = _dir.norm();
  Real theta = acos(_dir(2) / r);
  Real aphi = atan2(_dir(1), _dir(0));
  // rows of transformation matrix
  RealVectorValue x(
      cos(-aphi) * cos(M_PI / 2 - theta), sin(-aphi), -cos(-aphi) * sin(M_PI / 2 - theta));
  RealVectorValue y(
      -sin(-aphi) * cos(M_PI / 2 - theta), cos(-aphi), sin(-aphi) * sin(M_PI / 2 - theta));
  RealVectorValue z(sin(M_PI / 2 - theta), 0.0, cos(M_PI / 2 - theta));
  RealTensorValue R(x, y, z);

  for (unsigned int node_id = _first_node_id; node_id < _last_node_id; ++node_id)
  {
    Node & current_node = _phys_mesh->nodeRef(node_id);
    RealVectorValue p(current_node(0), current_node(1), current_node(2));
    // move to the origin, rotate about x-axis, transform to follow the direction and move to its
    // position
    current_node = R * (Rx * (p + _offset - z_offset)) + _position;
  }
}

const std::vector<RELAP7::Connection> &
GeometricalComponent::getConnections(RELAP7::EEndType id) const
{
  std::map<RELAP7::EEndType, std::vector<RELAP7::Connection>>::const_iterator it =
      _connections.find(id);
  if (it != _connections.end())
    return it->second;
  else
    mooseError(name(), ": No end of this type available (", id, ").");
}

void
GeometricalComponent::generateNodeLocations()
{
  unsigned int start_node = 0;
  Real start_length = 0.0;
  _node_locations = std::vector<Real>(_n_nodes);
  _node_locations[0] = start_length;

  for (unsigned int i = 0; i < _n_sections; ++i)
  {
    Real section_length = _lengths[i];
    Real section_n_elems = _n_elems[i];
    Real section_n_nodes = computeNumberOfNodes(section_n_elems);

    std::vector<Real> section_node_array = getUniformNodeLocations(section_length, section_n_nodes);
    placeLocalNodeLocations(start_length, start_node, section_node_array);

    start_length += section_length;
    start_node += (section_n_nodes - 1);
  }
}

std::vector<Real>
GeometricalComponent::getUniformNodeLocations(Real length, unsigned int n_nodes)
{
  std::vector<Real> node_locations(n_nodes);
  Real dx = length / (n_nodes - 1);

  node_locations[0] = 0.0;

  for (unsigned int i = 1; i < (n_nodes - 1); ++i)
    node_locations[i] = node_locations[i - 1] + dx;

  node_locations[n_nodes - 1] = length;
  return node_locations;
}

void
GeometricalComponent::placeLocalNodeLocations(Real start_length,
                                              unsigned int start_node,
                                              std::vector<Real> & local_node_locations)
{
  unsigned int n_nodes = local_node_locations.size();
  for (unsigned int i = 1; i < n_nodes; ++i)
  {
    unsigned int global_i = i + start_node;
    Real local_node_location = local_node_locations[i];
    _node_locations[global_i] = start_length + local_node_location;
  }
}

const FunctionName &
GeometricalComponent::getVariableFn(const FunctionName & fn_param_name)
{
  const FunctionName & fn_name = getParam<FunctionName>(fn_param_name);
  const Function & fn = _sim.getFunction(fn_name);

  if (dynamic_cast<const ConstantFunction *>(&fn) != nullptr)
  {
    connectObject(fn.parameters(), "", fn_name, fn_param_name, "value");
  }

  return fn_name;
}
