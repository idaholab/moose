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
   _thermal_conductivity_old(declarePropertyOld<Real>("thermal_conductivity"))
{}

void
StatefulTest::initQpStatefulProperties()
{
  _thermal_conductivity[_qp] = 0.;
}

void
StatefulTest::computeQpProperties()
{
  _thermal_conductivity[_qp] = _thermal_conductivity_old[_qp] + 1;
}
