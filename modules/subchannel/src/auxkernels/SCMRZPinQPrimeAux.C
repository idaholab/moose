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

#include "SCMRZPinQPrimeAux.h"

registerMooseObject("MooseApp", SCMRZPinQPrimeAux);

InputParameters
SCMRZPinQPrimeAux ::validParams()
{
  InputParameters params = DiffusionFluxAux::validParams();
  params.addClassDescription(
      "Axial heat rate on pin surface for a 2D-RZ axi-symmetric fuel pin model");
  return params;
}

SCMRZPinQPrimeAux ::SCMRZPinQPrimeAux(const InputParameters & parameters)
  : DiffusionFluxAux(parameters)
{
}

Real
SCMRZPinQPrimeAux ::computeValue()
{
  return DiffusionFluxAux::computeValue() * M_PI * 2.0 * abs(_q_point[_qp](0));
}
