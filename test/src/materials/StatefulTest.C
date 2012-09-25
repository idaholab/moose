#include "StatefulTest.h"

template<>
InputParameters validParams<StatefulTest>()
{
  InputParameters params = validParams<Material>();
  return params;
}

StatefulTest::StatefulTest(const std::string & name, InputParameters parameters)
  :Material(name, parameters),
   _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
   _thermal_conductivity_old(declarePropertyOld<Real>("thermal_conductivity")),
   _thermal_conductivity_older(declarePropertyOlder<Real>("thermal_conductivity"))
{}

void
StatefulTest::initQpStatefulProperties()
{
  _thermal_conductivity[_qp] = 1.0;
}

void
StatefulTest::computeQpProperties()
{
  // Really Expensive Fibonacci sequence generator!
  _thermal_conductivity[_qp] = _thermal_conductivity_old[_qp] + _thermal_conductivity_older[_qp];
}
