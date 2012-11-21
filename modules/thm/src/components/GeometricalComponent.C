#include "GeometricalComponent.h"

template<>
InputParameters validParams<GeometricalComponent>()
{
  InputParameters params = validParams<Component>();

  params.addRequiredParam<std::vector<Real> >("position", "Origin (start) of the pipe");
  params.addRequiredParam<std::vector<Real> >("orientation", "Orientation vector of the pipe");

  return params;
}

GeometricalComponent::GeometricalComponent(const std::string & name, InputParameters parameters) :
    Component(name, parameters),
    _position(toPoint(getParam<std::vector<Real> >("position")))
{
  const std::vector<Real> & dir = getParam<std::vector<Real> >("orientation");
  _dir = VectorValue<Real>(dir[0], dir[1], dir[2]);
}

GeometricalComponent::~GeometricalComponent()
{
}
