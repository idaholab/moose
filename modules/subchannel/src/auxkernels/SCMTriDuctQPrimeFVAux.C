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

#include "SCMTriDuctQPrimeFVAux.h"

registerMooseObject("MooseApp", SCMTriDuctQPrimeFVAux);

InputParameters
SCMTriDuctQPrimeFVAux::validParams()
{
  InputParameters params = DiffusionFluxFVAux::validParams();
  params.addClassDescription("Axial heat rate on duct surface");
  params.addRequiredParam<Real>("flat_to_flat", "[m]");
  return params;
}

SCMTriDuctQPrimeFVAux::SCMTriDuctQPrimeFVAux(const InputParameters & parameters)
  : DiffusionFluxFVAux(parameters), _flat_to_flat(getParam<Real>("flat_to_flat"))
{
}

Real
SCMTriDuctQPrimeFVAux::computeValue()
{
  return DiffusionFluxFVAux::computeValue() * 6 * _flat_to_flat / std::sqrt(3);
}
