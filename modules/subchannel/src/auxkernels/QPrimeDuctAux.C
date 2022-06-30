#include "QPrimeDuctAux.h"

registerMooseObject("MooseApp", QPrimeDuctAux);

InputParameters
QPrimeDuctAux::validParams()
{
  InputParameters params = DiffusionFluxAux::validParams();
  params.addClassDescription("Axial heat rate on duct surface");
  params.addRequiredParam<Real>("flat_to_flat", "[m]");
  return params;
}

QPrimeDuctAux::QPrimeDuctAux(const InputParameters & parameters)
  : DiffusionFluxAux(parameters), _flat_to_flat(getParam<Real>("flat_to_flat"))
{
}

Real
QPrimeDuctAux::computeValue()
{
  return DiffusionFluxAux::computeValue() * _flat_to_flat * std::tan(libMesh::pi / 6) * 2 * 6;
}
