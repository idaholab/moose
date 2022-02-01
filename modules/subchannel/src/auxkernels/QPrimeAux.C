#include "QPrimeAux.h"

registerMooseObject("MooseApp", QPrimeAux);

InputParameters
QPrimeAux::validParams()
{
  InputParameters params = DiffusionFluxAux::validParams();
  params.addClassDescription("Axial heat rate on pin surface");
  params.addRequiredParam<Real>("rod_diameter", "[m]");
  return params;
}

QPrimeAux::QPrimeAux(const InputParameters & parameters)
  : DiffusionFluxAux(parameters), _rod_diameter(getParam<Real>("rod_diameter"))
{
}

Real
QPrimeAux::computeValue()
{
  return DiffusionFluxAux::computeValue() * M_PI * _rod_diameter;
}
