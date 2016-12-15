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

  return params;
}

GeometricalComponent::GeometricalComponent(const InputParameters & parameters) :
    Component(parameters),
    _position(getParam<Point>("position")),
    _offset(getParam<RealVectorValue>("offset")),
    _dir(getParam<RealVectorValue>("orientation")),
    _rotation(getParam<Real>("rotation"))
{
}

GeometricalComponent::~GeometricalComponent()
{
}

void
GeometricalComponent::doBuildMesh()
{
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
