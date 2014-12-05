#include "GeometricalComponent.h"
#include "Conversion.h"

template<>
InputParameters validParams<GeometricalComponent>()
{
  InputParameters params = validParams<Component>();

  params.addRequiredParam<std::vector<Real> >("position", "Origin (start) of the pipe");
  params.addRequiredParam<std::vector<Real> >("orientation", "Orientation vector of the pipe");
  params.addParam<Real>("rotation", 0., "Rotation of the component (in degrees)");

  return params;
}

GeometricalComponent::GeometricalComponent(const std::string & name, InputParameters parameters) :
    Component(name, parameters),
    _position(toPoint(getParam<std::vector<Real> >("position"))),
    _rotation(getParam<Real>("rotation"))
{
  const std::vector<Real> & dir = getParam<std::vector<Real> >("orientation");
  _dir = VectorValue<Real>(dir[0], dir[1], dir[2]);
}

GeometricalComponent::~GeometricalComponent()
{
}

const std::vector<RELAP7::Connection> &
GeometricalComponent::getConnections(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, std::vector<RELAP7::Connection> >::iterator it = _connections.find(id);
  if (it != _connections.end())
    return it->second;
  else
    mooseError(name() << ": No end of this type available (" << id << ").");
}
