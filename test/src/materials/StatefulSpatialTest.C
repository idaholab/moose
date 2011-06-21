#include "StatefulSpatialTest.h"

template<>
InputParameters validParams<StatefulSpatialTest>()
{
  InputParameters params = validParams<Material>();
  return params;
}

StatefulSpatialTest::StatefulSpatialTest(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_old(declarePropertyOld<Real>("thermal_conductivity"))
{}

void
StatefulSpatialTest::computeQpProperties()
{
  if (_t_step == 1)
  {
    _thermal_conductivity[_qp] = _t_step + (_q_point[_qp](0) * _q_point[_qp](1));
  }
  else
  {
    _thermal_conductivity[_qp] = _thermal_conductivity_old[_qp] + 1.;
  }
}
