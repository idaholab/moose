//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumElems.h"
#include "SubProblem.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", NumElems);

InputParameters
NumElems::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum filt("active total", "active");
  params.addParam<MooseEnum>(
      "elem_filter",
      filt,
      "The type of elements to include in the count (active, total). Default == active");

  params.addClassDescription("Return the number of active or total elements in the simulation.");
  return params;
}

NumElems::NumElems(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _filt(parameters.get<MooseEnum>("elem_filter").getEnum<ElemFilter>()),
    _mesh(_subproblem.mesh().getMesh())
{
}

Real
NumElems::getValue()
{
  if (_filt == ElemFilter::ACTIVE)
    return _mesh.n_active_elem();
  return _mesh.n_elem();
}
