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

#include "SCMTriDuctQPrimeAux.h"

registerMooseObject("MooseApp", SCMTriDuctQPrimeAux);

InputParameters
SCMTriDuctQPrimeAux::validParams()
{
  InputParameters params = DiffusionFluxAux::validParams();
  params.addClassDescription("Axial heat rate on duct surface");
  params.addRequiredParam<Real>("flat_to_flat", "[m]");
  return params;
}

SCMTriDuctQPrimeAux::SCMTriDuctQPrimeAux(const InputParameters & parameters)
  : DiffusionFluxAux(parameters), _flat_to_flat(getParam<Real>("flat_to_flat"))
{
}

Real
SCMTriDuctQPrimeAux::computeValue()
{
  return DiffusionFluxAux::computeValue() * 6 * _flat_to_flat / std::sqrt(3);
}
