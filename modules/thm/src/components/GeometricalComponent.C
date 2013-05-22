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

Node *
GeometricalComponent::getBoundaryNode(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, Node *>::iterator it = _bnd_nodes.find(id);
  if (it != _bnd_nodes.end())
    return it->second;
  else
    return NULL;
}

unsigned int
GeometricalComponent::getBoundaryId(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, unsigned int>::iterator it = _bnd_ids.find(id);
  if (it != _bnd_ids.end())
    return it->second;
  else
    mooseError("Component " << name() << " does not have this type of end defined.");
}

Real
GeometricalComponent::getBoundaryOutNorm(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, Real>::iterator it = _bnd_out_norm.find(id);
  if (it != _bnd_out_norm.end())
    return it->second;
  else
    mooseError("Component " << name() << " does not have this type of end defined.");
  return 0;
}
