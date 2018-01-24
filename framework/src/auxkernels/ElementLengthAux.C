/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "ElementLengthAux.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ElementLengthAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam(
      "method", MooseEnum("min max"), "The size calculation to perform ('min' or 'max').");
  params.addClassDescription(
      "Compute the element size using Elem::hmin() or Elem::hmax() from libMesh.");
  return params;
}

ElementLengthAux::ElementLengthAux(const InputParameters & parameters)
  : AuxKernel(parameters), _use_min(getParam<MooseEnum>("method") == "min")
{
}

Real
ElementLengthAux::computeValue()
{
  if (_use_min)
    return _current_elem->hmin();
  return _current_elem->hmax();
}
