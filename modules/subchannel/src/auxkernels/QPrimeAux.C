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
