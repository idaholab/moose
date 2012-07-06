#include "ComputingInitialTest.h"

template<>
InputParameters validParams<ComputingInitialTest>()
{
  InputParameters params = validParams<Material>();
  return params;
}

ComputingInitialTest::ComputingInitialTest(const std::string & name, InputParameters parameters)
  :Material(name, parameters),
   _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
   _thermal_conductivity_old(declarePropertyOld<Real>("thermal_conductivity"))
{}

void
ComputingInitialTest::initQpStatefulProperties()
{
  _thermal_conductivity[_qp] = 0.;
}

void
ComputingInitialTest::computeQpProperties()
{
  if(_problem.computingInitialResidual())
    _thermal_conductivity[_qp] = _thermal_conductivity_old[_qp] + 1;
}
