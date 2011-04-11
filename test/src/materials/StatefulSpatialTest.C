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
StatefulSpatialTest::computeProperties()
{
  if (_t_step == 1)
  {
    for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
      _thermal_conductivity[qp] = _t_step + (_q_point[qp](0) * _q_point[qp](1));
  }
  else
  {
    for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
      _thermal_conductivity[qp] = _thermal_conductivity_old[qp] + 1.;
  }
}
