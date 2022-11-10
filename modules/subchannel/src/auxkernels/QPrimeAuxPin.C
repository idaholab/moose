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

#include "QPrimeAuxPin.h"

registerMooseObject("MooseApp", QPrimeAuxPin);

InputParameters
QPrimeAuxPin::validParams()
{
  InputParameters params = DiffusionFluxAux::validParams();
  params.addClassDescription("Axial heat rate on pin surface");
  params.addRequiredParam<Real>("rod_diameter", "[m]");
  return params;
}

QPrimeAuxPin::QPrimeAuxPin(const InputParameters & parameters)
  : DiffusionFluxAux(parameters), _rod_diameter(getParam<Real>("rod_diameter"))
{
}

Real
QPrimeAuxPin::computeValue()
{
  return DiffusionFluxAux::computeValue() * M_PI * _rod_diameter;
}
