/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "QPrimeDuctFVAux.h"

registerMooseObject("MooseApp", QPrimeDuctFVAux);

InputParameters
QPrimeDuctFVAux::validParams()
{
  InputParameters params = DiffusionFluxFVAux::validParams();
  params.addClassDescription("Axial heat rate on duct surface");
  params.addRequiredParam<Real>("flat_to_flat", "[m]");
  return params;
}

QPrimeDuctFVAux::QPrimeDuctFVAux(const InputParameters & parameters)
  : DiffusionFluxFVAux(parameters), _flat_to_flat(getParam<Real>("flat_to_flat"))
{
}

Real
QPrimeDuctFVAux::computeValue()
{
  return DiffusionFluxFVAux::computeValue() * 6 * _flat_to_flat / std::sqrt(3);
}
