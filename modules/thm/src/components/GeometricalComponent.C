#include "GeometricalComponent.h"
#include "Conversion.h"
#include "ConstantFunction.h"

template <>
InputParameters
validParams<GeometricalComponent>()
{
  InputParameters params = validParams<Component>();

  params.addRequiredParam<Point>("position", "Origin (start) of the pipe");
  params.addRequiredParam<RealVectorValue>("orientation", "Orientation vector of the pipe");
  params.addParam<Real>("rotation", 0., "Rotation of the component (in degrees)");
  params.addRequiredParam<std::vector<Real>>(
      "length", "The lengths of the subsections of the geometric component along the main axis");
  params.addRequiredParam<std::vector<unsigned int>>(
      "n_elems", "The number of elements in each subsection along the main axis");
  params.addParam<bool>("2nd_order_mesh", false, "Use 2nd order elements in the mesh");
  params.addRequiredParam<MooseEnum>("spatial_discretization",
                                     FlowModel::getSpatialDiscretizationMooseEnum("CG"),
                                     "Spatial discretization");

  return params;
}

GeometricalComponent::GeometricalComponent(const InputParameters & parameters)
  : Component(parameters),
    _position(getParam<Point>("position")),
    _dir(getParam<RealVectorValue>("orientation")),
    _gravity_angle(MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0)
                       ? 0.0
                       : std::acos(_dir * _gravity_vector / (_dir.norm() * _gravity_magnitude)) *
                             180 / M_PI),
    _rotation(getParam<Real>("rotation")),
    _lengths(getParam<std::vector<Real>>("length")),
    _length(std::accumulate(_lengths.begin(), _lengths.end(), 0.0)),
    _n_elems(getParam<std::vector<unsigned int>>("n_elems")),
    _n_elem(std::accumulate(_n_elems.begin(), _n_elems.end(), 0)),
    _2nd_order_mesh(getParam<bool>("2nd_order_mesh")),
    _spatial_discretization(
        getEnumParam<FlowModel::ESpatialDiscretizationType>("spatial_discretization")),
    _n_nodes(computeNumberOfNodes(_n_elem)),
    _n_sections(_lengths.size()),
    _fe_type(_spatial_discretization == FlowModel::rDG
                 ? FEType(CONSTANT, MONOMIAL)
                 : (_2nd_order_mesh ? FEType(SECOND, LAGRANGE) : FEType(FIRST, LAGRANGE))),
    _displace_node_user_object_name(genName(name(), "displace_node"))
{
  checkSizeGreaterThan<Real>("length", 0);
  checkEqualSize<Real, unsigned int>("length", "n_elems");
}

unsigned int
GeometricalComponent::computeNumberOfNodes(unsigned int n_elems)
{
  return _2nd_order_mesh ? (2 * n_elems) + 1 : n_elems + 1;
}

void
GeometricalComponent::computeMeshTransformation()
{
  Real rotation_rad = M_PI * _rotation / 180.;
  // rows of rotation matrix to rotate about x-axis
  RealVectorValue Rx_x(1., 0., 0.);
  RealVectorValue Rx_y(0., cos(rotation_rad), -sin(rotation_rad));
  RealVectorValue Rx_z(0., sin(rotation_rad), cos(rotation_rad));
  _Rx = RealTensorValue(Rx_x, Rx_y, Rx_z);

  // figure out the rotation
  Real r = _dir.norm();
  Real theta = acos(_dir(2) / r);
  Real aphi = atan2(_dir(1), _dir(0));
  // rows of transformation matrix
  RealVectorValue x(
      cos(-aphi) * cos(M_PI / 2. - theta), sin(-aphi), -cos(-aphi) * sin(M_PI / 2. - theta));
  RealVectorValue y(
      -sin(-aphi) * cos(M_PI / 2. - theta), cos(-aphi), sin(-aphi) * sin(M_PI / 2. - theta));
  RealVectorValue z(sin(M_PI / 2. - theta), 0.0, cos(M_PI / 2. - theta));
  _R = RealTensorValue(x, y, z);
}

void
GeometricalComponent::setupMesh()
{
  computeMeshTransformation();
  generateNodeLocations();
  unsigned int first_node_id = _mesh.nNodes();
  buildMesh();
  unsigned int last_node_id = _mesh.nNodes();

  // displace nodes
  for (unsigned int node_id = first_node_id; node_id < last_node_id; node_id++)
  {
    Node & curr_node = _mesh.nodeRef(node_id);
    RealVectorValue p(curr_node(0), curr_node(1), curr_node(2));
    curr_node = _R * (_Rx * p) + _position;
  }
}

void
GeometricalComponent::check() const
{
  Component::check();

  // Do not use TRAP q-rule with 2nd order FEs
  if (_2nd_order_mesh)
  {
    auto actions = _app.actionWarehouse().getActionListByName("setup_quadrature");
    const MooseEnum & quadrature_type = (*actions.begin())->getParam<MooseEnum>("type");

    if (quadrature_type == "TRAP")
      logError("Cannot use TRAP quadrature rule with 2nd order elements.  Use SIMPSON or GAUSS "
               "instead.");
  }
}

const std::vector<unsigned int> &
GeometricalComponent::getSubdomainIds() const
{
  checkSetupStatus(MESH_PREPARED);

  return _subdomain_ids;
}

const std::vector<SubdomainName> &
GeometricalComponent::getSubdomainNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _subdomain_names;
}

const std::vector<Moose::CoordinateSystemType> &
GeometricalComponent::getCoordSysTypes() const
{
  checkSetupStatus(MESH_PREPARED);

  return _coord_sys;
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

void
GeometricalComponent::setSubdomainInfo(unsigned int subdomain_id,
                                       const std::string & subdomain_name,
                                       const Moose::CoordinateSystemType & coord_system)
{
  _subdomain_ids.push_back(subdomain_id);
  _subdomain_names.push_back(subdomain_name);
  _coord_sys.push_back(coord_system);
  if (_parent)
  {
    GeometricalComponent * gc = dynamic_cast<GeometricalComponent *>(_parent);
    gc->_subdomain_ids.push_back(subdomain_id);
    gc->_subdomain_names.push_back(subdomain_name);
    gc->_coord_sys.push_back(coord_system);
  }
  _mesh.setSubdomainName(subdomain_id, subdomain_name);
}
