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
#include "NumElems.h"
#include "SubProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<NumElems>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  MooseEnum filt("active total", "active");
  params.addParam<MooseEnum>(
      "elem_filter",
      filt,
      "The type of elements to include in the count (active, total). Default == active");
  return params;
}

NumElems::NumElems(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _filt(parameters.get<MooseEnum>("elem_filter").getEnum<ElemFilter>())
{
}

Real
NumElems::getValue()
{
  if (_filt == ElemFilter::ACTIVE)
    return _subproblem.mesh().getMesh().n_active_elem();
  return _subproblem.mesh().nElem();
}
