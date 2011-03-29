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
StatefulTest::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {    
    _thermal_conductivity[qp] = _thermal_conductivity_old[qp] + 1;
  }
}
